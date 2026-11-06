/*******************************************************************/
/*                                                                 */
/*  DreamyDSP.h                                                    */
/*  Core Signal Processing & Visual Synthesizer Engine:            */
/*    1. 3-Band Audio Layer Analyzer (Bass, Mid, High)             */
/*    2. VHS Tape Wobble, Tracking Static & Chroma Bleed           */
/*    3. Frequency Modulation (FM) & Rutt-Etra Scanline Synth      */
/*    4. Circuit Bending (Phase Wavefolder, XOR DAC Shorts, Tear)  */
/*    5. 4-Stop Editable / Preset Gradient Colorizer               */
/*    6. CRT Scanlines & Trinitron RGB Phosphor Mask               */
/*                                                                 */
/*******************************************************************/

#ifndef DREAMY_DSP_H
#define DREAMY_DSP_H

#include "DreamyMusic.h"
#include <cmath>
#include <algorithm>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define DREAMY_HAS_NEON 1
#elif defined(__SSE2__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#define DREAMY_HAS_SSE 1
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DreamyDSP {

/* Fast deterministic hash for analog noise, tape static & circuit glitch (zero-fmod) */
inline float FastFract(float x) {
    return x - std::floor(x);
}

inline float Hash11(float p) {
    p = FastFract(p * 0.1031f);
    p *= p + 33.33f;
    p *= p + p;
    return FastFract(std::fabs(p));
}

inline float Hash21(float x, float y) {
    float p3x = FastFract(x * 0.1031f);
    float p3y = FastFract(y * 0.1030f);
    float p3z = FastFract(x * 0.0973f);
    float dot = p3x * (p3y + 33.33f) + p3y * (p3z + 33.33f) + p3z * (p3x + 33.33f);
    p3x += dot; p3y += dot; p3z += dot;
    return FastFract(std::fabs((p3x + p3y) * p3z));
}

inline float Clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

inline FloatRGBA ClampColor01(const FloatRGBA& c) {
#if defined(DREAMY_HAS_NEON)
    float32x4_t v = vld1q_f32(&c.r);
    float32x4_t zero = vdupq_n_f32(0.0f);
    float32x4_t one = vdupq_n_f32(1.0f);
    v = vmaxq_f32(v, zero);
    v = vminq_f32(v, one);
    FloatRGBA res;
    vst1q_f32(&res.r, v);
    return res;
#elif defined(DREAMY_HAS_SSE)
    __m128 v = _mm_loadu_ps(&c.r);
    __m128 zero = _mm_setzero_ps();
    __m128 one = _mm_set1_ps(1.0f);
    v = _mm_max_ps(v, zero);
    v = _mm_min_ps(v, one);
    FloatRGBA res;
    _mm_storeu_ps(&res.r, v);
    return res;
#else
    return {
        Clamp01(c.r),
        Clamp01(c.g),
        Clamp01(c.b),
        Clamp01(c.a)
    };
#endif
}

inline float SmoothStep(float edge0, float edge1, float x) {
    float t = Clamp01((x - edge0) / (edge1 - edge0 + 1e-6f));
    return t * t * (3.0f - 2.0f * t);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline FloatRGBA LerpColor(const FloatRGBA& a, const FloatRGBA& b, float t) {
#if defined(DREAMY_HAS_NEON)
    float32x4_t va = vld1q_f32(&a.r);
    float32x4_t vb = vld1q_f32(&b.r);
    float32x4_t diff = vsubq_f32(vb, va);
    FloatRGBA res;
    vst1q_f32(&res.r, vmlaq_n_f32(va, diff, t));
    return res;
#elif defined(DREAMY_HAS_SSE)
    __m128 va = _mm_loadu_ps(&a.r);
    __m128 vb = _mm_loadu_ps(&b.r);
    __m128 vt = _mm_set1_ps(t);
    __m128 diff = _mm_sub_ps(vb, va);
#if defined(__FMA__) || defined(__AVX2__)
    FloatRGBA res;
    _mm_storeu_ps(&res.r, _mm_fmadd_ps(diff, vt, va));
    return res;
#else
    FloatRGBA res;
    _mm_storeu_ps(&res.r, _mm_add_ps(va, _mm_mul_ps(diff, vt)));
    return res;
#endif
#else
    return {
        Lerp(a.r, b.r, t),
        Lerp(a.g, b.g, t),
        Lerp(a.b, b.b, t),
        Lerp(a.a, b.a, t)
    };
#endif
}

inline float Luminance(const FloatRGBA& c) {
    return 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
}

/*==================================================================
 * 1. BUILT-IN 3-BAND AUDIO LAYER ANALYZER + KEYFRAME SLIDER FUSION
 *==================================================================*/
inline void AnalyzeAudioReactivity(
    PF_InData* in_data,
    DreamyRenderParams& rp,
    float slider_val,
    float master_gain,
    float bass_weight,
    float mid_weight,
    float high_weight,
    float window_ms)
{
    float raw_bass = 0.0f;
    float raw_mid  = 0.0f;
    float raw_high = 0.0f;
    float peak_abs = 0.0f;
    rp.audio_checkout_err = PF_Err_NONE;
    rp.audio_num_samples  = 0;

    /* If music reactivity is disabled, bypass all audio checkout and filtering immediately */
    if (!rp.audio_enable) {
        rp.audio_raw_bass    = 0.0f;
        rp.audio_raw_mid     = 0.0f;
        rp.audio_raw_high    = 0.0f;
        rp.audio_peak_sample = 0.0f;
        rp.audio_bass        = 0.0f;
        rp.audio_mid         = 0.0f;
        rp.audio_high        = 0.0f;
        rp.audio_overall     = 0.0f;
        return;
    }

    /* 1. Attempt direct audio checkout from PARAM_AUDIO_LAYER */
    if (in_data->inter.checkout_layer_audio && in_data->inter.get_audio_data && in_data->time_scale > 0) {
        void* audio_handle = nullptr;
        PF_UFixed sample_rate = (PF_UFixed)(44100UL << 16); /* 0xAC440000 (PF_UFixed 44100.0 Hz) */
        A_long window_time = (A_long)((window_ms / 1000.0f) * (float)in_data->time_scale);
        if (window_time < (A_long)in_data->time_step) {
            window_time = in_data->time_step;
        }
        if (window_time <= 0) {
            window_time = (A_long)(in_data->time_scale / 30);
            if (window_time <= 0) window_time = 1;
        }
        A_long start_time = in_data->current_time;
        if (start_time < 0) start_time = 0;

        PF_Err audio_err = PF_CHECKOUT_LAYER_AUDIO(
            in_data,
            PARAM_AUDIO_LAYER,
            start_time,
            window_time,
            in_data->time_scale,
            sample_rate,
            PF_SSS_4,
            PF_Channels_MONO,
            PF_SIGNED_FLOAT,
            &audio_handle);
        rp.audio_checkout_err = audio_err;

        if (audio_err == PF_Err_NONE && audio_handle != nullptr) {
            PF_SoundSampleFloat* samples = nullptr;
            A_long num_samples = 0;

            if (PF_GET_AUDIO_DATA(
                    in_data,
                    audio_handle,
                    &samples,
                    &num_samples,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr) == PF_Err_NONE && samples != nullptr && num_samples > 0)
            {
                rp.audio_num_samples = num_samples;
                /* Cascaded 1-pole IIR filter bank at ~44.1kHz:
                 * Lowpass @ ~160Hz (Bass), Bandpass @ 160Hz-2.5kHz (Mid), Highpass > 2.5kHz (High) */
                float lp_bass = 0.0f;
                float lp_mid  = 0.0f;
                const float alpha_bass = 0.022f; /* ~160 Hz at 44.1kHz */
                const float alpha_mid  = 0.280f; /* ~2500 Hz at 44.1kHz */

                double sum_bass_sq = 0.0;
                double sum_mid_sq  = 0.0;
                double sum_high_sq = 0.0;

                A_long max_inspect = num_samples > 8192 ? 8192 : num_samples;
                for (A_long i = 0; i < max_inspect; ++i) {
                    float mono = samples[i];
                    /* Guard against NaN/Inf in audio buffer */
                    if (!(mono >= -4.0f && mono <= 4.0f)) mono = 0.0f;
                    float am = std::fabs(mono);
                    if (am > peak_abs) peak_abs = am;

                    lp_bass += alpha_bass * (mono - lp_bass);
                    lp_mid  += alpha_mid  * (mono - lp_mid);

                    float b_samp = lp_bass;
                    float m_samp = lp_mid - lp_bass;
                    float h_samp = mono - lp_mid;

                    sum_bass_sq += (double)(b_samp * b_samp);
                    sum_mid_sq  += (double)(m_samp * m_samp);
                    sum_high_sq += (double)(h_samp * h_samp);
                }

                /* Convert RMS energy to perceptual 0..1+ envelope */
                raw_bass = (float)std::sqrt(sum_bass_sq / (double)max_inspect) * 3.8f;
                raw_mid  = (float)std::sqrt(sum_mid_sq  / (double)max_inspect) * 4.5f;
                raw_high = (float)std::sqrt(sum_high_sq / (double)max_inspect) * 6.0f;
            }
            PF_CHECKIN_LAYER_AUDIO(in_data, audio_handle);
        }
    }

    /* 2. Fuse with Keyframable Audio Slider (0..100 scale, compatible with Convert Audio to Keyframes) */
    float norm_slider = slider_val * 0.025f; /* So typical AE Audio Amplitude (10-40) maps to 0.25-1.0 */
    if (norm_slider > 0.0001f) {
        /* Add rhythmic harmonic variations so slider alone still gives organic multi-band movement */
        float t = rp.time_sec;
        raw_bass += norm_slider * (0.95f + 0.15f * std::sin(t * 8.0f));
        raw_mid  += norm_slider * (0.85f + 0.15f * std::cos(t * 13.0f));
        raw_high += norm_slider * (0.75f + 0.25f * std::sin(t * 21.0f));
    }

    /* 3. Apply user band weights & master gain */
    rp.audio_raw_bass    = raw_bass;
    rp.audio_raw_mid     = raw_mid;
    rp.audio_raw_high    = raw_high;
    rp.audio_peak_sample = peak_abs;
    rp.audio_bass        = std::min(3.0f, raw_bass * bass_weight * master_gain);
    rp.audio_mid         = std::min(3.0f, raw_mid  * mid_weight  * master_gain);
    rp.audio_high        = std::min(3.0f, raw_high * high_weight * master_gain);
    rp.audio_overall     = (rp.audio_bass * 0.45f + rp.audio_mid * 0.35f + rp.audio_high * 0.20f);
}

/*==================================================================
 * 2. GRADIENT PALETTE SYNTHESIZER (PRESETS & CUSTOM 4-STOP)
 *==================================================================*/
inline void SetupGradientStops(
    DreamyRenderParams& rp,
    A_long mode,
    const PF_Pixel& c0,
    const PF_Pixel& c1,
    const PF_Pixel& c2,
    const PF_Pixel& c3)
{
    rp.gradient_mode = mode;
    switch (mode) {
        case GRAD_MODE_CUSTOM:
            rp.stops[0] = { c0.red / 255.0f, c0.green / 255.0f, c0.blue / 255.0f, 1.0f };
            rp.stops[1] = { c1.red / 255.0f, c1.green / 255.0f, c1.blue / 255.0f, 1.0f };
            rp.stops[2] = { c2.red / 255.0f, c2.green / 255.0f, c2.blue / 255.0f, 1.0f };
            rp.stops[3] = { c3.red / 255.0f, c3.green / 255.0f, c3.blue / 255.0f, 1.0f };
            break;

        case GRAD_MODE_SUNSET_DREAM:
            /* Deep night violet -> electric magenta -> fiery orange-gold -> pastel cyan-white */
            rp.stops[0] = { 0.08f, 0.02f, 0.14f, 1.0f };
            rp.stops[1] = { 0.88f, 0.15f, 0.65f, 1.0f };
            rp.stops[2] = { 1.00f, 0.56f, 0.10f, 1.0f };
            rp.stops[3] = { 0.72f, 0.90f, 1.00f, 1.0f };
            break;

        case GRAD_MODE_ACID_CIRCUIT:
            /* Deep cobalt blue -> acid lime green -> solar yellow -> hot neon magenta */
            rp.stops[0] = { 0.05f, 0.08f, 0.75f, 1.0f };
            rp.stops[1] = { 0.22f, 0.88f, 0.12f, 1.0f };
            rp.stops[2] = { 0.98f, 0.82f, 0.08f, 1.0f };
            rp.stops[3] = { 0.98f, 0.18f, 0.82f, 1.0f };
            break;

        case GRAD_MODE_NEON_OSCILLOSCOPE:
            /* Warm phosphor amber -> neon coral -> electric indigo -> laser cyan */
            rp.stops[0] = { 0.04f, 0.02f, 0.06f, 1.0f };
            rp.stops[1] = { 0.95f, 0.45f, 0.05f, 1.0f };
            rp.stops[2] = { 0.55f, 0.25f, 0.98f, 1.0f };
            rp.stops[3] = { 0.35f, 0.98f, 0.95f, 1.0f };
            break;

        case GRAD_MODE_CYBERPUNK_VHS:
            /* Deep midnight -> hot pink -> electric cyan -> warm cream */
            rp.stops[0] = { 0.03f, 0.05f, 0.18f, 1.0f };
            rp.stops[1] = { 0.92f, 0.08f, 0.52f, 1.0f };
            rp.stops[2] = { 0.05f, 0.88f, 0.96f, 1.0f };
            rp.stops[3] = { 1.00f, 0.96f, 0.86f, 1.0f };
            break;

        case GRAD_MODE_THERMAL_CRT:
            /* Void black -> deep crimson -> amber gold -> incandescent white */
            rp.stops[0] = { 0.02f, 0.01f, 0.04f, 1.0f };
            rp.stops[1] = { 0.78f, 0.06f, 0.18f, 1.0f };
            rp.stops[2] = { 0.98f, 0.68f, 0.08f, 1.0f };
            rp.stops[3] = { 1.00f, 0.98f, 0.92f, 1.0f };
            break;

        case GRAD_MODE_PASTEL_VAPORWAVE:
        default:
            /* Twilight plum -> dreamy rose -> mint aqua -> soft lavender white */
            rp.stops[0] = { 0.16f, 0.09f, 0.32f, 1.0f };
            rp.stops[1] = { 0.94f, 0.45f, 0.72f, 1.0f };
            rp.stops[2] = { 0.42f, 0.94f, 0.86f, 1.0f };
            rp.stops[3] = { 0.96f, 0.90f, 1.00f, 1.0f };
            break;
    }
}

/* Sample 4-stop cyclic gradient at normalized position t */
inline FloatRGBA EvaluateGradientDirect(const DreamyRenderParams& rp, float t) {
    /* Wrap t smoothly into [0, 1] with phase cycling */
    float p = t + rp.gradient_phase;
    p = p - std::floor(p);
    /* Triangle-wave wrap option when cycling so there's never a hard seam */
    if (std::fabs(rp.gradient_phase) > 0.001f) {
        float val = std::fabs(t + rp.gradient_phase);
        float tri = val - 2.0f * std::floor(val * 0.5f);
        p = tri > 1.0f ? (2.0f - tri) : tri;
    }
    p = Clamp01(p);

    if (p < 0.333333f) {
        return LerpColor(rp.stops[0], rp.stops[1], p * 3.0f);
    } else if (p < 0.666667f) {
        return LerpColor(rp.stops[1], rp.stops[2], (p - 0.333333f) * 3.0f);
    } else {
        return LerpColor(rp.stops[2], rp.stops[3], (p - 0.666667f) * 3.0f);
    }
}

/* O(1) LUT-accelerated gradient sampler (table populated once per frame) */
inline FloatRGBA SampleGradient(const DreamyRenderParams& rp, float t) {
    float ct = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    int idx = (int)(ct * 1023.0f + 0.5f);
    return rp.gradient_lut[idx];
}

/* Precompute all Frame-Invariant Constants & 1024-entry Gradient LUT */
inline void PrecomputeFrameConstants(DreamyRenderParams& rp) {
    rp.inv_width  = rp.width  > 0 ? (1.0f / (float)rp.width)  : 0.0f;
    rp.inv_height = rp.height > 0 ? (1.0f / (float)rp.height) : 0.0f;

    /* Per-target audio modulation weights */
    const bool audio_active = (rp.audio_enable != FALSE);
    const float fm_audio_react   = (audio_active && rp.audio_react_fm)       ? rp.fm_audio_react   : 0.0f;
    const float bend_audio_react = (audio_active && rp.audio_react_bend)     ? rp.bend_audio_react : 0.0f;
    const float vhs_bass         = (audio_active && rp.audio_react_vhs)      ? rp.audio_bass       : 0.0f;
    const float vhs_mid          = (audio_active && rp.audio_react_vhs)      ? rp.audio_mid        : 0.0f;
    const float echo_bass        = (audio_active && rp.audio_react_echo)     ? rp.audio_bass       : 0.0f;
    const float echo_mid         = (audio_active && rp.audio_react_echo)     ? rp.audio_mid        : 0.0f;

    /* FM & Rutt-Etra constants */
    rp.fm_audio_mult    = 1.0f + (rp.audio_bass * 1.1f + rp.audio_mid * 0.6f) * fm_audio_react;
    rp.fm_carrier_rad   = (rp.fm_carrier_freq * 0.5f) * (float)(2.0 * M_PI);
    rp.fm_mod_index     = rp.fm_mod_depth * rp.fm_audio_mult * 18.0f;
    rp.fm_x_shift_scale = (rp.fm_mod_depth * 14.0f * rp.fm_audio_mult) * rp.downsample_scale;
    rp.fm_z_elev        = rp.fm_rutt_etra_z * rp.fm_audio_mult;

    /* Circuit Bending constants */
    rp.bend_audio_mult = 1.0f + (rp.audio_high * 0.9f + rp.audio_bass * 0.6f) * bend_audio_react;
    float fb_angle     = (rp.bend_feedback_shift * rp.bend_audio_mult) * (float)M_PI * 1.5f;
    rp.bend_fb_cos     = std::cos(fb_angle);
    rp.bend_fb_sin     = std::sin(fb_angle);
    rp.bend_fold_gain  = 1.0f + rp.bend_phase_fold * rp.bend_audio_mult * 4.5f;
    rp.bend_crush_amt  = Clamp01(rp.bend_xor_crush * rp.bend_audio_mult);
    rp.bend_xor_mask   = (uint8_t)((int)(rp.bend_crush_amt * 170.0f) & 0xFC);

    /* VHS constants */
    rp.vhs_eff_wobble = rp.vhs_tape_wobble * (1.0f + vhs_bass * 1.35f);
    rp.vhs_eff_track  = rp.vhs_tracking_noise * (1.0f + vhs_bass * 0.8f);
    float bpos        = FastFract(0.88f - rp.time_sec * 0.12f);
    if (bpos < 0.0f) bpos += 1.0f;
    rp.vhs_bar_pos      = bpos;
    float eff_bleed     = rp.vhs_chroma_bleed * (1.0f + vhs_mid * 0.8f);
    rp.vhs_eff_bleed_px = eff_bleed > 0.001f ? (eff_bleed * 18.0f * rp.downsample_scale) : 0.0f;
    rp.vhs_eff_bloom    = rp.vhs_dream_bloom * (1.0f + vhs_mid * 0.9f + vhs_bass * 0.5f);
    rp.vhs_bloom_rad1   = (6.0f + rp.vhs_eff_bloom * 12.0f) * rp.downsample_scale;
    rp.vhs_bloom_rad2   = (14.0f + rp.vhs_eff_bloom * 26.0f) * rp.downsample_scale;

    /* CRT constants */
    rp.crt_pitch      = std::max(1.5f, rp.crt_scanline_pitch * rp.downsample_scale);
    rp.crt_inv_pitch  = 1.0f / rp.crt_pitch;
    rp.crt_cell_w     = std::max(1.0f, rp.crt_pitch * 0.5f);
    rp.crt_inv_cell_w = 1.0f / rp.crt_cell_w;

    /* Echo Trail precomputed per-step tables */
    rp.echo_active_count  = 0;
    rp.echo_glow_strength = rp.echo_glow * (1.0f + echo_mid * 0.5f);
    rp.echo_trail_mix     = Clamp01(rp.echo_opacity * 0.78f);

    if (rp.echo_enable && rp.echo_count > 0 && rp.echo_opacity > 0.001f) {
        int max_echoes = (int)std::min<A_long>(rp.echo_count, 12);
        float audio_trail_boost = 1.0f + (echo_bass * 0.45f + echo_mid * 0.45f);
        float step_dist = rp.echo_offset_px * audio_trail_boost * rp.downsample_scale;
        float decay_rate = Clamp01(rp.echo_decay);
        float glow_rad = (4.0f + rp.echo_glow * 14.0f) * rp.downsample_scale;

        float base_angle = rp.time_sec * 1.15f + echo_bass * 0.8f;
        float dir_x = std::cos(base_angle);
        float dir_y = std::sin(base_angle * 0.75f) * 0.65f;

        for (int i = 0; i < max_echoes; ++i) {
            float step_alpha = rp.echo_opacity * std::pow(1.0f - decay_rate * 0.78f, (float)i);
            if (step_alpha < 0.005f) break;

            const PF_EffectWorld* ew = (i < rp.echo_valid_worlds && rp.echo_worlds[i] && rp.echo_worlds[i]->data)
                                     ? rp.echo_worlds[i]
                                     : rp.input_world;
            rp.echo_step_deep[i]     = (ew && PF_WORLD_IS_DEEP(ew)) ? TRUE : FALSE;
            rp.echo_step_alpha[i]    = step_alpha;
            float step_idx           = (float)(i + 1);
            rp.echo_step_dx[i]       = -dir_x * step_dist * step_idx;
            float dy                 = -dir_y * step_dist * step_idx * 0.7f;
            if (rp.fm_enable && rp.fm_rutt_etra_z > 0.001f) {
                dy += step_idx * rp.fm_rutt_etra_z * 6.5f * rp.downsample_scale;
            }
            rp.echo_step_dy[i]       = dy;
            rp.echo_step_glow_rad[i] = glow_rad * (0.7f + 0.25f * step_idx);
            rp.echo_active_count     = i + 1;
        }
    }

    /* Populate 1024-entry Gradient Lookup Table */
    if (rp.gradient_enable && rp.gradient_mode != GRAD_MODE_OFF && rp.gradient_mix > 0.001f) {
        const float inv1023 = 1.0f / 1023.0f;
        for (int i = 0; i < 1024; ++i) {
            rp.gradient_lut[i] = EvaluateGradientDirect(rp, (float)i * inv1023);
        }
    }
}

/*==================================================================
 * 3. BILINEAR PIXEL SAMPLER (8-BIT & 16-BIT DEEP COLOR)
 *==================================================================*/
inline FloatRGBA SampleSpecificWorldBilinear(const PF_EffectWorld* w, A_Boolean is_16bit, float x, float y) {
    if (!w || !w->data || w->width <= 0 || w->height <= 0) {
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    /* Guard against NaN / Inf coordinates */
    if (!(x >= -1e7f && x <= 1e7f)) x = 0.0f;
    if (!(y >= -1e7f && y <= 1e7f)) y = 0.0f;

    /* Clamp coordinates to valid input layer bounds */
    float max_x = (float)(w->width - 1);
    float max_y = (float)(w->height - 1);
    float cx = x < 0.0f ? 0.0f : (x > max_x ? max_x : x);
    float cy = y < 0.0f ? 0.0f : (y > max_y ? max_y : y);

    A_long x0 = (A_long)cx;
    A_long y0 = (A_long)cy;
    A_long x1 = (x0 + 1 < w->width)  ? (x0 + 1) : (w->width - 1);
    A_long y1 = (y0 + 1 < w->height) ? (y0 + 1) : (w->height - 1);

    float fx = cx - (float)x0;
    float fy = cy - (float)y0;

    const char* base = reinterpret_cast<const char*>(w->data);

    if (is_16bit) {
        const PF_Pixel16* row0 = reinterpret_cast<const PF_Pixel16*>(base + y0 * w->rowbytes);
        const PF_Pixel16* row1 = reinterpret_cast<const PF_Pixel16*>(base + y1 * w->rowbytes);

        const PF_Pixel16& p00 = row0[x0];
        const PF_Pixel16& p10 = row0[x1];
        const PF_Pixel16& p01 = row1[x0];
        const PF_Pixel16& p11 = row1[x1];

        const float inv16 = 1.0f / 32768.0f;
        float omfx = 1.0f - fx;
        float omfy = (1.0f - fy) * inv16;
        float fyy  = fy * inv16;
        float w00  = omfx * omfy;
        float w10  = fx * omfy;
        float w01  = omfx * fyy;
        float w11  = fx * fyy;

#if defined(DREAMY_HAS_NEON)
        float32x4_t v00 = { (float)p00.red, (float)p00.green, (float)p00.blue, (float)p00.alpha };
        float32x4_t v10 = { (float)p10.red, (float)p10.green, (float)p10.blue, (float)p10.alpha };
        float32x4_t v01 = { (float)p01.red, (float)p01.green, (float)p01.blue, (float)p01.alpha };
        float32x4_t v11 = { (float)p11.red, (float)p11.green, (float)p11.blue, (float)p11.alpha };

        float32x4_t res = vmulq_n_f32(v00, w00);
        res = vmlaq_n_f32(res, v10, w10);
        res = vmlaq_n_f32(res, v01, w01);
        res = vmlaq_n_f32(res, v11, w11);

        FloatRGBA out;
        vst1q_f32(&out.r, res);
        return out;
#elif defined(DREAMY_HAS_SSE)
        __m128 v00 = _mm_set_ps((float)p00.alpha, (float)p00.blue, (float)p00.green, (float)p00.red);
        __m128 v10 = _mm_set_ps((float)p10.alpha, (float)p10.blue, (float)p10.green, (float)p10.red);
        __m128 v01 = _mm_set_ps((float)p01.alpha, (float)p01.blue, (float)p01.green, (float)p01.red);
        __m128 v11 = _mm_set_ps((float)p11.alpha, (float)p11.blue, (float)p11.green, (float)p11.red);

        __m128 vw00 = _mm_set1_ps(w00);
        __m128 vw10 = _mm_set1_ps(w10);
        __m128 vw01 = _mm_set1_ps(w01);
        __m128 vw11 = _mm_set1_ps(w11);

#if defined(__FMA__) || defined(__AVX2__)
        __m128 res = _mm_mul_ps(v00, vw00);
        res = _mm_fmadd_ps(v10, vw10, res);
        res = _mm_fmadd_ps(v01, vw01, res);
        res = _mm_fmadd_ps(v11, vw11, res);
#else
        __m128 res = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v00, vw00), _mm_mul_ps(v10, vw10)),
            _mm_add_ps(_mm_mul_ps(v01, vw01), _mm_mul_ps(v11, vw11))
        );
#endif
        FloatRGBA out;
        _mm_storeu_ps(&out.r, res);
        return out;
#else
        return {
            p00.red   * w00 + p10.red   * w10 + p01.red   * w01 + p11.red   * w11,
            p00.green * w00 + p10.green * w10 + p01.green * w01 + p11.green * w11,
            p00.blue  * w00 + p10.blue  * w10 + p01.blue  * w01 + p11.blue  * w11,
            p00.alpha * w00 + p10.alpha * w10 + p01.alpha * w01 + p11.alpha * w11
        };
#endif
    } else {
        const PF_Pixel8* row0 = reinterpret_cast<const PF_Pixel8*>(base + y0 * w->rowbytes);
        const PF_Pixel8* row1 = reinterpret_cast<const PF_Pixel8*>(base + y1 * w->rowbytes);

        const PF_Pixel8& p00 = row0[x0];
        const PF_Pixel8& p10 = row0[x1];
        const PF_Pixel8& p01 = row1[x0];
        const PF_Pixel8& p11 = row1[x1];

        const float inv8 = 1.0f / 255.0f;
        float omfx = 1.0f - fx;
        float omfy = (1.0f - fy) * inv8;
        float fyy  = fy * inv8;
        float w00  = omfx * omfy;
        float w10  = fx * omfy;
        float w01  = omfx * fyy;
        float w11  = fx * fyy;

#if defined(DREAMY_HAS_NEON)
        float32x4_t v00 = { (float)p00.red, (float)p00.green, (float)p00.blue, (float)p00.alpha };
        float32x4_t v10 = { (float)p10.red, (float)p10.green, (float)p10.blue, (float)p10.alpha };
        float32x4_t v01 = { (float)p01.red, (float)p01.green, (float)p01.blue, (float)p01.alpha };
        float32x4_t v11 = { (float)p11.red, (float)p11.green, (float)p11.blue, (float)p11.alpha };

        float32x4_t res = vmulq_n_f32(v00, w00);
        res = vmlaq_n_f32(res, v10, w10);
        res = vmlaq_n_f32(res, v01, w01);
        res = vmlaq_n_f32(res, v11, w11);

        FloatRGBA out;
        vst1q_f32(&out.r, res);
        return out;
#elif defined(DREAMY_HAS_SSE)
        __m128 v00 = _mm_set_ps((float)p00.alpha, (float)p00.blue, (float)p00.green, (float)p00.red);
        __m128 v10 = _mm_set_ps((float)p10.alpha, (float)p10.blue, (float)p10.green, (float)p10.red);
        __m128 v01 = _mm_set_ps((float)p01.alpha, (float)p01.blue, (float)p01.green, (float)p01.red);
        __m128 v11 = _mm_set_ps((float)p11.alpha, (float)p11.blue, (float)p11.green, (float)p11.red);

        __m128 vw00 = _mm_set1_ps(w00);
        __m128 vw10 = _mm_set1_ps(w10);
        __m128 vw01 = _mm_set1_ps(w01);
        __m128 vw11 = _mm_set1_ps(w11);

#if defined(__FMA__) || defined(__AVX2__)
        __m128 res = _mm_mul_ps(v00, vw00);
        res = _mm_fmadd_ps(v10, vw10, res);
        res = _mm_fmadd_ps(v01, vw01, res);
        res = _mm_fmadd_ps(v11, vw11, res);
#else
        __m128 res = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v00, vw00), _mm_mul_ps(v10, vw10)),
            _mm_add_ps(_mm_mul_ps(v01, vw01), _mm_mul_ps(v11, vw11))
        );
#endif
        FloatRGBA out;
        _mm_storeu_ps(&out.r, res);
        return out;
#else
        return {
            p00.red   * w00 + p10.red   * w10 + p01.red   * w01 + p11.red   * w11,
            p00.green * w00 + p10.green * w10 + p01.green * w01 + p11.green * w11,
            p00.blue  * w00 + p10.blue  * w10 + p01.blue  * w01 + p11.blue  * w11,
            p00.alpha * w00 + p10.alpha * w10 + p01.alpha * w01 + p11.alpha * w11
        };
#endif
    }
}

inline FloatRGBA SampleWorldBilinear(const DreamyRenderParams& rp, float x, float y) {
    return SampleSpecificWorldBilinear(rp.input_world, rp.is_16bit, x, y);
}

/*==================================================================
 * 4. ROW-INVARIANT PRECOMPUTATION & PIXEL SYNTHESIS PIPELINE
 *==================================================================*/
struct DreamyRowState {
    A_long    py;
    float     v0;
    float     sample_y0;
    bool      has_curvature;
    float     cv0;
    float     cv0_sq;
    float     crt_k;
    float     crt_vig_strength;

    /* Fast-path row constants when CRT curvature is 0 */
    float     flat_dx_offset;
    float     flat_vhs_bar_env;
    float     flat_snow_y;

    /* Row-invariant CRT Scanline & Subpixel Phosphor Mask tables */
    float     crt_scan_wave;
    float     crt_sqrt_scan_wave;
    float     crt_mask_r[6];
    float     crt_mask_g[6];
    float     crt_mask_b[6];
};

inline void PrecomputeRowState(const DreamyRenderParams& rp, A_long py, DreamyRowState& rs) {
    rs.py               = py;
    rs.sample_y0        = (float)py + 0.5f;
    rs.v0               = rs.sample_y0 * rp.inv_height;
    rs.has_curvature    = (rp.crt_enable && rp.crt_curvature > 0.001f);
    rs.cv0              = rs.v0 * 2.0f - 1.0f;
    rs.cv0_sq           = rs.cv0 * rs.cv0;
    rs.crt_k            = rp.crt_curvature * 0.22f;
    rs.crt_vig_strength = rp.crt_curvature * 0.65f;

    rs.flat_dx_offset   = 0.0f;
    rs.flat_vhs_bar_env = 0.0f;
    rs.flat_snow_y      = std::floor(rs.sample_y0) + rp.time_sec * 120.0f;

    /* When CRT curvature is off, v and sample_y are constant across the entire row */
    if (!rs.has_curvature) {
        float v = rs.v0;
        float sample_y = rs.sample_y0;

        if (rp.vhs_enable) {
            if (rp.vhs_eff_wobble > 0.001f) {
                float hum        = std::sin(v * 10.0f + rp.time_sec * 9.0f) * 3.5f;
                float flutter    = std::sin(v * 75.0f - rp.time_sec * 37.0f) * 1.2f;
                float line_noise = (Hash21(std::floor(sample_y), rp.time_sec * 12.0f) - 0.5f) * 4.0f;
                rs.flat_dx_offset += (hum + flutter + line_noise) * rp.vhs_eff_wobble * rp.downsample_scale;
            }
            if (rp.vhs_eff_track > 0.001f) {
                float dist    = std::fabs(v - rp.vhs_bar_pos);
                float bar_env = SmoothStep(0.08f * rp.vhs_eff_track, 0.0f, dist);
                rs.flat_vhs_bar_env = bar_env;
                if (bar_env > 0.001f) {
                    float tear = (Hash21(std::floor(sample_y * 0.5f), rp.time_sec * 60.0f) - 0.5f)
                               * 45.0f * bar_env * rp.vhs_eff_track;
                    rs.flat_dx_offset += tear * rp.downsample_scale;
                }
            }
        }

        if (rp.bend_enable && rp.bend_smear_tear > 0.001f) {
            float eff_tear  = rp.bend_smear_tear * rp.bend_audio_mult;
            float band_id   = std::floor(v * 28.0f + std::sin(rp.time_sec * 4.0f) * 3.0f);
            float band_hash = Hash21(band_id, std::floor(rp.time_sec * 15.0f));
            if (band_hash > (1.0f - std::min(0.85f, eff_tear * 0.65f))) {
                float tear_offset = (band_hash - 0.5f) * 110.0f * eff_tear * rp.downsample_scale;
                rs.flat_dx_offset += tear_offset * std::sin(v * 140.0f + rp.time_sec * 20.0f);
            }
        }
    }

    /* Precompute row-invariant CRT scanline wave & 6-state RGB phosphor multipliers */
    rs.crt_scan_wave      = 1.0f;
    rs.crt_sqrt_scan_wave = 1.0f;
    if (rp.crt_enable) {
        if (rp.crt_scanline_intensity > 0.001f) {
            float scan_phase      = ((float)py * rp.crt_inv_pitch) * (float)(2.0 * M_PI);
            rs.crt_scan_wave      = 0.5f + 0.5f * std::cos(scan_phase);
            rs.crt_sqrt_scan_wave = std::sqrt(rs.crt_scan_wave);
        }

        if (rp.crt_phosphor_mask != CRT_MASK_NONE && rp.crt_phosphor_strength > 0.001f) {
            int sub_y = (int)std::floor((float)py * rp.crt_inv_pitch);
            int stagger = (rp.crt_phosphor_mask == CRT_MASK_SHADOW_TRIAD && (sub_y & 1)) ? 1 : 0;
            float str = rp.crt_phosphor_strength;

            for (int m = 0; m < 6; ++m) {
                int sx = m + stagger;
                int triad_idx = sx % 3;
                float mr, mg, mb;
                if (triad_idx == 0) {
                    mr = 1.48f; mg = 0.15f; mb = 0.12f;
                } else if (triad_idx == 1) {
                    mr = 0.15f; mg = 1.48f; mb = 0.15f;
                } else {
                    mr = 0.12f; mg = 0.15f; mb = 1.48f;
                }

                if (rp.crt_phosphor_mask == CRT_MASK_SLOT_MASK) {
                    float slot_v = FastFract(((float)py + ((sx & 1) ? rp.crt_pitch * 0.5f : 0.0f)) * rp.crt_inv_pitch);
                    if (slot_v > 0.82f) {
                        mr *= 0.45f; mg *= 0.45f; mb *= 0.45f;
                    }
                }

                rs.crt_mask_r[m] = Lerp(1.0f, mr, str);
                rs.crt_mask_g[m] = Lerp(1.0f, mg, str);
                rs.crt_mask_b[m] = Lerp(1.0f, mb, str);
            }
        } else {
            for (int m = 0; m < 6; ++m) {
                rs.crt_mask_r[m] = 1.0f;
                rs.crt_mask_g[m] = 1.0f;
                rs.crt_mask_b[m] = 1.0f;
            }
        }
    }
}

inline FloatRGBA SynthesizePixel(const DreamyRenderParams& rp, const DreamyRowState& rs, A_long px, A_long py) {
    const float w = (float)rp.width;
    const float h = (float)rp.height;
    float u = ((float)px + 0.5f) * rp.inv_width;
    float v = rs.v0;

    /*--------------------------------------------------------------
     * STAGE A: CRT Glass Curvature (Barrel Distortion)
     *--------------------------------------------------------------*/
    float vignette = 1.0f;
    if (rs.has_curvature) {
        float cu = u * 2.0f - 1.0f;
        float cv = rs.cv0;
        float r2 = cu * cu + rs.cv0_sq;
        float scale = 1.0f + rs.crt_k * r2;
        cu *= scale;
        cv *= scale;

        /* Soft CRT corner bezel darkening */
        if (std::fabs(cu) > 1.0f || std::fabs(cv) > 1.0f) {
            return { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        u = cu * 0.5f + 0.5f;
        v = cv * 0.5f + 0.5f;
        vignette = 1.0f - SmoothStep(0.75f, 1.05f, std::sqrt(r2) * 0.75f) * rs.crt_vig_strength;
    }

    float sample_x = u * w;
    float sample_y = rs.has_curvature ? (v * h) : rs.sample_y0;

    /*--------------------------------------------------------------
     * STAGE B & C: VHS Tape Wobble, Tracking Bar & Circuit Bend Tear
     *--------------------------------------------------------------*/
    float tracking_static_add = 0.0f;
    if (!rs.has_curvature) {
        /* Fast row-invariant path */
        sample_x += rs.flat_dx_offset;
        if (rs.flat_vhs_bar_env > 0.001f) {
            float snow = Hash21(std::floor(sample_x * 0.5f), rs.flat_snow_y);
            if (snow > 0.45f) {
                tracking_static_add = (snow - 0.45f) * 1.8f * rs.flat_vhs_bar_env * rp.vhs_eff_track;
            }
        }
    } else {
        if (rp.vhs_enable) {
            if (rp.vhs_eff_wobble > 0.001f) {
                /* 60Hz mains hum wave + high-frequency head switching jitter */
                float hum        = std::sin(v * 10.0f + rp.time_sec * 9.0f) * 3.5f;
                float flutter    = std::sin(v * 75.0f - rp.time_sec * 37.0f) * 1.2f;
                float line_noise = (Hash21(std::floor(sample_y), rp.time_sec * 12.0f) - 0.5f) * 4.0f;
                sample_x += (hum + flutter + line_noise) * rp.vhs_eff_wobble * rp.downsample_scale;
            }

            /* VHS Tracking Noise Bar (scrolls vertically and flares on bass hits) */
            if (rp.vhs_eff_track > 0.001f) {
                float dist    = std::fabs(v - rp.vhs_bar_pos);
                float bar_env = SmoothStep(0.08f * rp.vhs_eff_track, 0.0f, dist);
                if (bar_env > 0.001f) {
                    /* Strong horizontal tear inside the tracking band */
                    float tear = (Hash21(std::floor(sample_y * 0.5f), rp.time_sec * 60.0f) - 0.5f)
                               * 45.0f * bar_env * rp.vhs_eff_track;
                    sample_x += tear * rp.downsample_scale;
                    float snow = Hash21(std::floor(sample_x * 0.5f), std::floor(sample_y) + rp.time_sec * 120.0f);
                    if (snow > 0.45f) {
                        tracking_static_add = (snow - 0.45f) * 1.8f * bar_env * rp.vhs_eff_track;
                    }
                }
            }
        }

        if (rp.bend_enable && rp.bend_smear_tear > 0.001f) {
            float eff_tear  = rp.bend_smear_tear * rp.bend_audio_mult;
            /* Quantized vertical band hold error */
            float band_id   = std::floor(v * 28.0f + std::sin(rp.time_sec * 4.0f) * 3.0f);
            float band_hash = Hash21(band_id, std::floor(rp.time_sec * 15.0f));
            if (band_hash > (1.0f - std::min(0.85f, eff_tear * 0.65f))) {
                float tear_offset = (band_hash - 0.5f) * 110.0f * eff_tear * rp.downsample_scale;
                /* Smooth saw wave inside torn scanline band */
                sample_x += tear_offset * std::sin(v * 140.0f + rp.time_sec * 20.0f);
            }
        }
    }

    /*--------------------------------------------------------------
     * STAGE D: Frequency Modulation (FM) & Rutt-Etra Scanline Synth
     * (Glowing 3D-displaced FM oscilloscope waves)
     *--------------------------------------------------------------*/
    FloatRGBA base_col = SampleWorldBilinear(rp, sample_x, sample_y);
    float local_luma = Luminance(base_col);
    float rutt_ribbon_glow = 0.0f;
    float fm_carrier_signal = 0.0f;

    if (rp.fm_enable) {
        /* 1. True Phase/Frequency Modulation:
         * Brightness modulates the phase of a high-frequency carrier wave */
        float smooth_luma = local_luma;
        if (rp.fm_mod_depth > 0.001f || rp.fm_z_elev > 0.001f) {
            float dx_tap = 3.0f * rp.downsample_scale;
            float luma_left   = Luminance(SampleWorldBilinear(rp, sample_x - dx_tap, sample_y));
            float luma_right  = Luminance(SampleWorldBilinear(rp, sample_x + dx_tap, sample_y));
            smooth_luma = 0.25f * luma_left + 0.5f * local_luma + 0.25f * luma_right;
        }

        /* Horizontal FM displacement + carrier harmonic texture */
        float fm_phase = u * rp.fm_carrier_rad + smooth_luma * rp.fm_mod_index + rp.time_sec * 6.0f;
        fm_carrier_signal = std::sin(fm_phase);

        if (rp.fm_x_shift_scale > 0.001f) {
            sample_x += fm_carrier_signal * rp.fm_x_shift_scale;
        }

        /* 2. Rutt-Etra 3D Scanline Z-Elevation & Glowing Oscilloscope Ribbons:
         * In a Rutt-Etra video synthesizer, each horizontal scanline is deflected
         * vertically (Y) proportional to image luminance (Z) + FM wave modulation. */
        float z_elev = rp.fm_z_elev;
        if (z_elev > 0.001f || rp.fm_scanline_density > 0.001f) {
            /* Vertical deflection of sampling coordinate by luminance + subtle FM ripple */
            float y_deflect = (smooth_luma * 55.0f + fm_carrier_signal * 4.5f) * z_elev * rp.downsample_scale;
            sample_y += y_deflect;

            /* Re-sample at the FM + Rutt-Etra deflected coordinates */
            base_col = SampleWorldBilinear(rp, sample_x, sample_y);
            local_luma = Luminance(base_col);

            /* Generate glowing horizontal oscilloscope contour lines */
            if (rp.fm_scanline_density > 1.0f) {
                float num_lines = rp.fm_scanline_density;
                /* Deformed scanline coordinate combines vertical position, luminance elevation, and FM wave */
                float deformed_v = v * num_lines
                                 - (local_luma * z_elev * (num_lines * 0.16f))
                                 + (fm_carrier_signal * rp.fm_mod_depth * 0.45f * rp.fm_audio_mult);

                /* Distance to nearest scanline center (0.0 at center, 0.5 at midpoint between lines) */
                float frac_line = FastFract(deformed_v);
                float dist_to_line = std::fabs(frac_line - 0.5f) * 2.0f; /* 0 at center, 1 at boundary */

                /* Beam thickness & phosphor bloom around each Rutt-Etra line */
                float beam_width = Clamp01(rp.fm_line_thickness);
                float core_beam  = SmoothStep(beam_width, beam_width * 0.15f, dist_to_line);

                /* High-frequency dotted/stippled FM beam modulation along the line */
                float dot_mod = 0.82f + 0.18f * fm_carrier_signal;
                rutt_ribbon_glow = core_beam * dot_mod;

                /* Darken gaps between scanlines proportional to Rutt-Etra Z depth for authentic oscilloscope look */
                float gap_mask = Lerp(1.0f, 0.12f + 0.88f * core_beam, Clamp01(z_elev * 0.85f));
                base_col.r *= gap_mask;
                base_col.g *= gap_mask;
                base_col.b *= gap_mask;
            }
        } else {
            base_col = SampleWorldBilinear(rp, sample_x, sample_y);
            local_luma = Luminance(base_col);
        }
    }

    /*--------------------------------------------------------------
     * STAGE E: Dreamy VHS Chroma Bleed & Halation Bloom
     *--------------------------------------------------------------*/
    if (rp.vhs_enable) {
        /* 1. Analog YIQ/YUV Chroma Delay & Horizontal Color Bleed */
        if (rp.vhs_eff_bleed_px > 0.018f) {
            float shift_px = rp.vhs_eff_bleed_px;
            FloatRGBA col_left  = SampleWorldBilinear(rp, sample_x - shift_px, sample_y);
            FloatRGBA col_right = SampleWorldBilinear(rp, sample_x + shift_px, sample_y);
            FloatRGBA col_far   = SampleWorldBilinear(rp, sample_x - shift_px * 1.8f, sample_y);

            /* Keep sharp luminance (Y), smear Red/Blue chroma channels horizontally */
            float y_orig = Luminance(base_col);
            base_col.r = 0.45f * base_col.r + 0.35f * col_left.r + 0.20f * col_far.r;
            base_col.b = 0.45f * base_col.b + 0.55f * col_right.b;
            base_col.g = 0.70f * base_col.g + 0.15f * col_left.g + 0.15f * col_right.g;

            /* Restore luma detail while preserving dreamy chroma halo */
            float y_new = Luminance(base_col);
            float y_diff = (y_orig - y_new) * 0.55f;
            base_col.r = Clamp01(base_col.r + y_diff);
            base_col.g = Clamp01(base_col.g + y_diff);
            base_col.b = Clamp01(base_col.b + y_diff);
        }

        /* 2. Dreamy Halation & Soft Multi-Tap Highlight Bloom */
        if (rp.vhs_eff_bloom > 0.001f) {
            float rad1 = rp.vhs_bloom_rad1;
            float rad2_diag = rp.vhs_bloom_rad2 * 0.7f;

            /* 8-tap dreamy star/halo kernel around current sample */
            FloatRGBA b1 = SampleWorldBilinear(rp, sample_x + rad1, sample_y);
            FloatRGBA b2 = SampleWorldBilinear(rp, sample_x - rad1, sample_y);
            FloatRGBA b3 = SampleWorldBilinear(rp, sample_x, sample_y + rad1);
            FloatRGBA b4 = SampleWorldBilinear(rp, sample_x, sample_y - rad1);
            FloatRGBA b5 = SampleWorldBilinear(rp, sample_x + rad2_diag, sample_y + rad2_diag);
            FloatRGBA b6 = SampleWorldBilinear(rp, sample_x - rad2_diag, sample_y + rad2_diag);
            FloatRGBA b7 = SampleWorldBilinear(rp, sample_x + rad2_diag, sample_y - rad2_diag);
            FloatRGBA b8 = SampleWorldBilinear(rp, sample_x - rad2_diag, sample_y - rad2_diag);

#if defined(DREAMY_HAS_NEON)
            float32x4_t vb1 = vld1q_f32(&b1.r);
            float32x4_t vb2 = vld1q_f32(&b2.r);
            float32x4_t vb3 = vld1q_f32(&b3.r);
            float32x4_t vb4 = vld1q_f32(&b4.r);
            float32x4_t vb5 = vld1q_f32(&b5.r);
            float32x4_t vb6 = vld1q_f32(&b6.r);
            float32x4_t vb7 = vld1q_f32(&b7.r);
            float32x4_t vb8 = vld1q_f32(&b8.r);

            float32x4_t card = vaddq_f32(vaddq_f32(vb1, vb2), vaddq_f32(vb3, vb4));
            float32x4_t diag = vaddq_f32(vaddq_f32(vb5, vb6), vaddq_f32(vb7, vb8));
            float32x4_t vbloom = vmlaq_n_f32(vmulq_n_f32(card, 0.15f), diag, 0.10f);

            FloatRGBA avg_halo;
            vst1q_f32(&avg_halo.r, vbloom);
            avg_halo.a = 1.0f;
#elif defined(DREAMY_HAS_SSE)
            __m128 vb1 = _mm_loadu_ps(&b1.r);
            __m128 vb2 = _mm_loadu_ps(&b2.r);
            __m128 vb3 = _mm_loadu_ps(&b3.r);
            __m128 vb4 = _mm_loadu_ps(&b4.r);
            __m128 vb5 = _mm_loadu_ps(&b5.r);
            __m128 vb6 = _mm_loadu_ps(&b6.r);
            __m128 vb7 = _mm_loadu_ps(&b7.r);
            __m128 vb8 = _mm_loadu_ps(&b8.r);

            __m128 card = _mm_add_ps(_mm_add_ps(vb1, vb2), _mm_add_ps(vb3, vb4));
            __m128 diag = _mm_add_ps(_mm_add_ps(vb5, vb6), _mm_add_ps(vb7, vb8));
            __m128 vcard_w = _mm_set1_ps(0.15f);
            __m128 vdiag_w = _mm_set1_ps(0.10f);

#if defined(__FMA__) || defined(__AVX2__)
            __m128 vbloom = _mm_fmadd_ps(diag, vdiag_w, _mm_mul_ps(card, vcard_w));
#else
            __m128 vbloom = _mm_add_ps(_mm_mul_ps(card, vcard_w), _mm_mul_ps(diag, vdiag_w));
#endif
            FloatRGBA avg_halo;
            _mm_storeu_ps(&avg_halo.r, vbloom);
            avg_halo.a = 1.0f;
#else
            FloatRGBA avg_halo = {
                (b1.r + b2.r + b3.r + b4.r) * 0.15f + (b5.r + b6.r + b7.r + b8.r) * 0.10f,
                (b1.g + b2.g + b3.g + b4.g) * 0.15f + (b5.g + b6.g + b7.g + b8.g) * 0.10f,
                (b1.b + b2.b + b3.b + b4.b) * 0.15f + (b5.b + b6.b + b7.b + b8.b) * 0.10f,
                1.0f
            };
#endif

            /* Highlight knee extraction + dreamy warm/magenta halation tint */
            float halo_luma = Luminance(avg_halo);
            float highlight_boost = SmoothStep(0.18f, 0.90f, halo_luma) * rp.vhs_eff_bloom * 0.85f;

            /* Screen-like soft additive blend */
            base_col.r = 1.0f - (1.0f - base_col.r) * (1.0f - Clamp01(avg_halo.r * highlight_boost * 1.15f));
            base_col.g = 1.0f - (1.0f - base_col.g) * (1.0f - Clamp01(avg_halo.g * highlight_boost * 0.95f));
            base_col.b = 1.0f - (1.0f - base_col.b) * (1.0f - Clamp01(avg_halo.b * highlight_boost * 1.20f));
        }
    }

    /*--------------------------------------------------------------
     * STAGE E2: Dreamy Echo Trail (Windows Cursor-Trail Style
     * Semi-Transparent Ghosting + Customizable Trail Glow)
     *--------------------------------------------------------------*/
    if (rp.echo_active_count > 0) {
        FloatRGBA trail_accum = { 0.0f, 0.0f, 0.0f, 0.0f };
        float halo_sum_r = 0.0f, halo_sum_g = 0.0f, halo_sum_b = 0.0f;
        float g_env_sum  = 0.0f;
        float weight_sum = 0.0f;
        const float glow_strength = rp.echo_glow_strength;
        const bool do_glow = (glow_strength > 0.001f);

        for (A_long i = 0; i < rp.echo_active_count; ++i) {
            const PF_EffectWorld* ew = (i < rp.echo_valid_worlds && rp.echo_worlds[i] && rp.echo_worlds[i]->data)
                                     ? rp.echo_worlds[i]
                                     : rp.input_world;
            A_Boolean ew_deep = rp.echo_step_deep[i];
            float step_alpha  = rp.echo_step_alpha[i];

            /* Precomputed stepping offset + Rutt-Etra echo phase lag */
            float ox = sample_x + rp.echo_step_dx[i];
            float oy = sample_y + rp.echo_step_dy[i];

            FloatRGBA ghost = SampleSpecificWorldBilinear(ew, ew_deep, ox, oy);
            float ghost_luma = Luminance(ghost);
            /* Emphasize moving highlights & edges so the trail floats semi-transparently over the background */
            float presence = SmoothStep(0.05f, 0.75f, ghost_luma) * ghost.a;
            float w_ghost  = step_alpha * (0.35f + 0.65f * presence);

            trail_accum.r += ghost.r * w_ghost;
            trail_accum.g += ghost.g * w_ghost;
            trail_accum.b += ghost.b * w_ghost;
            weight_sum    += w_ghost;

            /* Customizable Echo Trail Glow: 4-tap halo around visible ghost copies */
            if (do_glow && (step_alpha * glow_strength > 0.012f)) {
                float r_i = rp.echo_step_glow_rad[i];
                FloatRGBA g1 = SampleSpecificWorldBilinear(ew, ew_deep, ox + r_i, oy);
                FloatRGBA g2 = SampleSpecificWorldBilinear(ew, ew_deep, ox - r_i, oy);
                FloatRGBA g3 = SampleSpecificWorldBilinear(ew, ew_deep, ox, oy + r_i);
                FloatRGBA g4 = SampleSpecificWorldBilinear(ew, ew_deep, ox, oy - r_i);

                float hr = (g1.r + g2.r + g3.r + g4.r) * 0.25f;
                float hg = (g1.g + g2.g + g3.g + g4.g) * 0.25f;
                float hb = (g1.b + g2.b + g3.b + g4.b) * 0.25f;
                float halo_luma = 0.299f * hr + 0.587f * hg + 0.114f * hb;
                float g_env = SmoothStep(0.12f, 0.85f, halo_luma) * step_alpha * glow_strength * 0.45f;

                halo_sum_r += hr * g_env;
                halo_sum_g += hg * g_env;
                halo_sum_b += hb * g_env;
                g_env_sum  += g_env;
            }
        }

        if (weight_sum > 0.001f) {
            float inv_w = 1.0f / weight_sum;
            float avg_r = trail_accum.r * inv_w;
            float avg_g = trail_accum.g * inv_w;
            float avg_b = trail_accum.b * inv_w;

            /* Semi-transparent cursor-trail composite: blend ghosts with translucent screen/alpha mix */
            float trail_mix = rp.echo_trail_mix;
            float screen_r = 1.0f - (1.0f - base_col.r) * (1.0f - Clamp01(avg_r * trail_mix));
            float screen_g = 1.0f - (1.0f - base_col.g) * (1.0f - Clamp01(avg_g * trail_mix));
            float screen_b = 1.0f - (1.0f - base_col.b) * (1.0f - Clamp01(avg_b * trail_mix));

            base_col.r = Lerp(Lerp(base_col.r, avg_r, trail_mix * 0.55f), screen_r, 0.65f);
            base_col.g = Lerp(Lerp(base_col.g, avg_g, trail_mix * 0.55f), screen_g, 0.65f);
            base_col.b = Lerp(Lerp(base_col.b, avg_b, trail_mix * 0.55f), screen_b, 0.65f);
        }

        /* Add the customizable Echo Trail Glow aura (tinted once after accumulating all ghosts) */
        if (g_env_sum > 0.0001f) {
            float glow_r = halo_sum_r * (0.25f + 1.05f * rp.echo_glow_color.r) + (0.2625f * rp.echo_glow_color.r) * g_env_sum;
            float glow_g = halo_sum_g * (0.25f + 1.05f * rp.echo_glow_color.g) + (0.2625f * rp.echo_glow_color.g) * g_env_sum;
            float glow_b = halo_sum_b * (0.25f + 1.05f * rp.echo_glow_color.b) + (0.2625f * rp.echo_glow_color.b) * g_env_sum;

            base_col.r = 1.0f - (1.0f - base_col.r) * (1.0f - Clamp01(glow_r));
            base_col.g = 1.0f - (1.0f - base_col.g) * (1.0f - Clamp01(glow_g));
            base_col.b = 1.0f - (1.0f - base_col.b) * (1.0f - Clamp01(glow_b));
        }
    }

    /*--------------------------------------------------------------
     * STAGE F: Circuit Bending (Analog Wavefolding & DAC Pin XOR)
     * (Psychedelic color wrapping & glitch)
     *--------------------------------------------------------------*/
    if (rp.bend_enable) {
        /* 1. Cross-channel RGB feedback matrix rotation (using frame-precomputed sin/cos) */
        if (rp.bend_feedback_shift > 0.001f) {
            float ca = rp.bend_fb_cos;
            float sa = rp.bend_fb_sin * 0.6f;
            float nr = base_col.r * ca - base_col.g * sa + base_col.b * sa;
            float ng = base_col.g * ca + base_col.r * sa - base_col.b * sa;
            float nb = base_col.b * ca - base_col.r * sa + base_col.g * sa;
            base_col.r = Clamp01(std::fabs(nr));
            base_col.g = Clamp01(std::fabs(ng));
            base_col.b = Clamp01(std::fabs(nb));
        }

        /* 2. Analog Video Synth Phase Wavefolder / Solarization (Tachyons+ style) */
        if (rp.bend_phase_fold > 0.001f) {
            float fold_gain = rp.bend_fold_gain;
            /* Different phase offsets per channel create psychedelic rainbow contours */
            base_col.r = 0.5f + 0.5f * std::sin((base_col.r * fold_gain - 0.25f) * (float)M_PI);
            base_col.g = 0.5f + 0.5f * std::sin((base_col.g * fold_gain * 1.12f - 0.10f) * (float)M_PI);
            base_col.b = 0.5f + 0.5f * std::sin((base_col.b * fold_gain * 1.25f + 0.15f) * (float)M_PI);
        }

        /* 3. Digital DAC Pin Short / Bitwise XOR Quantization */
        if (rp.bend_xor_crush > 0.001f) {
            float crush_mix = rp.bend_crush_amt * 0.85f;
            uint8_t r8 = (uint8_t)(Clamp01(base_col.r) * 255.0f);
            uint8_t g8 = (uint8_t)(Clamp01(base_col.g) * 255.0f);
            uint8_t b8 = (uint8_t)(Clamp01(base_col.b) * 255.0f);

            /* Emulate shorted data pins on an 8-bit video ADC/DAC */
            uint8_t mask = rp.bend_xor_mask;
            uint8_t xr = (r8 ^ ((g8 >> 1) & mask));
            uint8_t xg = (g8 ^ ((b8 >> 1) & mask));
            uint8_t xb = (b8 ^ ((r8 >> 1) & mask));

            const float inv255 = 1.0f / 255.0f;
            base_col.r = Lerp(base_col.r, xr * inv255, crush_mix);
            base_col.g = Lerp(base_col.g, xg * inv255, crush_mix);
            base_col.b = Lerp(base_col.b, xb * inv255, crush_mix);
        }
    }

    /*--------------------------------------------------------------
     * STAGE G: 4-Stop Editable / Preset Gradient Colorizer (O(1) LUT)
     *--------------------------------------------------------------*/
    if (rp.gradient_enable && rp.gradient_mode != GRAD_MODE_OFF && rp.gradient_mix > 0.001f) {
        /* Map pixel luminance + FM wave harmonic + Rutt-Etra ribbon intensity through the 1024-entry LUT */
        float synth_luma = Luminance(base_col);
        if (rp.fm_enable) {
            synth_luma = Clamp01(synth_luma + rutt_ribbon_glow * 0.28f + fm_carrier_signal * rp.fm_mod_depth * 0.12f);
        }

        FloatRGBA grad_col = SampleGradient(rp, synth_luma);

        /* If Rutt-Etra scanlines or circuit bending are active, allow subtle per-channel gradient offset
         * so chroma separation shines through the gradient palette */
        if ((rp.vhs_enable && rp.vhs_chroma_bleed > 0.01f) || (rp.bend_enable && rp.bend_phase_fold > 0.01f)) {
            FloatRGBA grad_r = SampleGradient(rp, base_col.r);
            FloatRGBA grad_g = SampleGradient(rp, base_col.g);
            FloatRGBA grad_b = SampleGradient(rp, base_col.b);
            grad_col.r = Lerp(grad_col.r, grad_r.r, 0.45f);
            grad_col.g = Lerp(grad_col.g, grad_g.g, 0.45f);
            grad_col.b = Lerp(grad_col.b, grad_b.b, 0.45f);
        }

        /* Boost Rutt-Etra glowing ribbon cores toward the bright highlight stop */
        if (rutt_ribbon_glow > 0.01f) {
            grad_col = LerpColor(grad_col, rp.stops[3], rutt_ribbon_glow * 0.35f);
        }

        base_col.r = Lerp(base_col.r, grad_col.r, rp.gradient_mix);
        base_col.g = Lerp(base_col.g, grad_col.g, rp.gradient_mix);
        base_col.b = Lerp(base_col.b, grad_col.b, rp.gradient_mix);
    } else if (rp.fm_enable && rutt_ribbon_glow > 0.01f) {
        /* When gradient colors are turned off, preserve original footage colors while keeping subtle Rutt-Etra beam glow */
        float boost = 1.0f + rutt_ribbon_glow * 0.22f;
        base_col.r = Clamp01(base_col.r * boost);
        base_col.g = Clamp01(base_col.g * boost);
        base_col.b = Clamp01(base_col.b * boost);
    }

    /*--------------------------------------------------------------
     * STAGE H: VHS Magnetic Tape Grain & Tracking Static Overlay
     *--------------------------------------------------------------*/
    if (rp.vhs_enable) {
        if (tracking_static_add > 0.001f) {
            base_col.r = Clamp01(base_col.r + tracking_static_add);
            base_col.g = Clamp01(base_col.g + tracking_static_add * 0.95f);
            base_col.b = Clamp01(base_col.b + tracking_static_add * 1.05f);
        }
        if (rp.vhs_grain > 0.001f) {
            float n = (Hash21((float)px + rp.time_sec * 131.0f, (float)py + rp.time_sec * 73.0f) - 0.5f) * rp.vhs_grain * 0.35f;
            base_col.r = Clamp01(base_col.r + n);
            base_col.g = Clamp01(base_col.g + n * 0.9f);
            base_col.b = Clamp01(base_col.b + n * 1.1f);
        }
    }

    /*--------------------------------------------------------------
     * STAGE I: CRT Scanlines & Trinitron RGB Phosphor Subpixel Mask
     * (Uses precomputed row scanline wave & 6-state RGB phosphor table)
     *--------------------------------------------------------------*/
    if (rp.crt_enable) {
        /* 1. Horizontal CRT Scanline Beam Profile */
        if (rp.crt_scanline_intensity > 0.001f) {
            /* Bright pixels bloom wider across scanline gaps on real CRTs */
            float pix_luma   = Luminance(base_col);
            float bloom_comp = Lerp(rs.crt_scan_wave, rs.crt_sqrt_scan_wave, pix_luma * 0.65f);
            float scan_mult  = Lerp(1.0f, bloom_comp * 1.15f, rp.crt_scanline_intensity);

            base_col.r *= scan_mult;
            base_col.g *= scan_mult;
            base_col.b *= scan_mult;
        }

        /* 2. RGB Phosphor Aperture Grille / Shadow Mask / Slot Mask */
        if (rp.crt_phosphor_mask != CRT_MASK_NONE && rp.crt_phosphor_strength > 0.001f) {
            int sub_x = (int)((float)px * rp.crt_inv_cell_w);
            int mod6  = sub_x % 6;
            base_col.r *= rs.crt_mask_r[mod6];
            base_col.g *= rs.crt_mask_g[mod6];
            base_col.b *= rs.crt_mask_b[mod6];
        }

        /* Apply CRT glass corner vignette */
        base_col.r *= vignette;
        base_col.g *= vignette;
        base_col.b *= vignette;
    }

    base_col.r = Clamp01(base_col.r);
    base_col.g = Clamp01(base_col.g);
    base_col.b = Clamp01(base_col.b);
    base_col.a = Clamp01(base_col.a);
    return base_col;
}

inline FloatRGBA SynthesizePixel(const DreamyRenderParams& rp, A_long px, A_long py) {
    DreamyRowState rs;
    PrecomputeRowState(rp, py, rs);
    return SynthesizePixel(rp, rs, px, py);
}

} /* namespace DreamyDSP */

#endif /* DREAMY_DSP_H */
