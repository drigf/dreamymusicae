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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DreamyDSP {

/* Fast deterministic hash for analog noise, tape static & circuit glitch */
inline float Hash11(float p) {
    p = std::fmod(p * 0.1031f, 1.0f);
    p *= p + 33.33f;
    p *= p + p;
    return std::fmod(std::fabs(p), 1.0f);
}

inline float Hash21(float x, float y) {
    float p3x = std::fmod(x * 0.1031f, 1.0f);
    float p3y = std::fmod(y * 0.1030f, 1.0f);
    float p3z = std::fmod(x * 0.0973f, 1.0f);
    float dot = p3x * (p3y + 33.33f) + p3y * (p3z + 33.33f) + p3z * (p3x + 33.33f);
    p3x += dot; p3y += dot; p3z += dot;
    return std::fmod(std::fabs((p3x + p3y) * p3z), 1.0f);
}

inline float Clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

inline float SmoothStep(float edge0, float edge1, float x) {
    float t = Clamp01((x - edge0) / (edge1 - edge0 + 1e-6f));
    return t * t * (3.0f - 2.0f * t);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline FloatRGBA LerpColor(const FloatRGBA& a, const FloatRGBA& b, float t) {
    return {
        Lerp(a.r, b.r, t),
        Lerp(a.g, b.g, t),
        Lerp(a.b, b.b, t),
        Lerp(a.a, b.a, t)
    };
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
            /* Reference Image 1: Deep night violet -> electric magenta -> fiery orange-gold -> pastel cyan-white */
            rp.stops[0] = { 0.08f, 0.02f, 0.14f, 1.0f };
            rp.stops[1] = { 0.88f, 0.15f, 0.65f, 1.0f };
            rp.stops[2] = { 1.00f, 0.56f, 0.10f, 1.0f };
            rp.stops[3] = { 0.72f, 0.90f, 1.00f, 1.0f };
            break;

        case GRAD_MODE_ACID_CIRCUIT:
            /* Reference Image 2: Deep cobalt blue -> acid lime green -> solar yellow -> hot neon magenta */
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
inline FloatRGBA SampleGradient(const DreamyRenderParams& rp, float t) {
    /* Wrap t smoothly into [0, 1] with phase cycling */
    float p = t + rp.gradient_phase;
    p = p - std::floor(p);
    /* Triangle-wave wrap option when cycling so there's never a hard seam */
    if (std::fabs(rp.gradient_phase) > 0.001f) {
        float tri = std::fmod(std::fabs(t + rp.gradient_phase), 2.0f);
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

/*==================================================================
 * 3. BILINEAR PIXEL SAMPLER (8-BIT & 16-BIT DEEP COLOR)
 *==================================================================*/
inline FloatRGBA SampleWorldBilinear(const DreamyRenderParams& rp, float x, float y) {
    const PF_EffectWorld* w = rp.input_world;
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

    if (rp.is_16bit) {
        const PF_Pixel16* row0 = reinterpret_cast<const PF_Pixel16*>(base + y0 * w->rowbytes);
        const PF_Pixel16* row1 = reinterpret_cast<const PF_Pixel16*>(base + y1 * w->rowbytes);

        const PF_Pixel16& p00 = row0[x0];
        const PF_Pixel16& p10 = row0[x1];
        const PF_Pixel16& p01 = row1[x0];
        const PF_Pixel16& p11 = row1[x1];

        const float inv16 = 1.0f / 32768.0f;
        float w00 = (1.0f - fx) * (1.0f - fy) * inv16;
        float w10 = fx * (1.0f - fy) * inv16;
        float w01 = (1.0f - fx) * fy * inv16;
        float w11 = fx * fy * inv16;

        return {
            p00.red   * w00 + p10.red   * w10 + p01.red   * w01 + p11.red   * w11,
            p00.green * w00 + p10.green * w10 + p01.green * w01 + p11.green * w11,
            p00.blue  * w00 + p10.blue  * w10 + p01.blue  * w01 + p11.blue  * w11,
            p00.alpha * w00 + p10.alpha * w10 + p01.alpha * w01 + p11.alpha * w11
        };
    } else {
        const PF_Pixel8* row0 = reinterpret_cast<const PF_Pixel8*>(base + y0 * w->rowbytes);
        const PF_Pixel8* row1 = reinterpret_cast<const PF_Pixel8*>(base + y1 * w->rowbytes);

        const PF_Pixel8& p00 = row0[x0];
        const PF_Pixel8& p10 = row0[x1];
        const PF_Pixel8& p01 = row1[x0];
        const PF_Pixel8& p11 = row1[x1];

        const float inv8 = 1.0f / 255.0f;
        float w00 = (1.0f - fx) * (1.0f - fy) * inv8;
        float w10 = fx * (1.0f - fy) * inv8;
        float w01 = (1.0f - fx) * fy * inv8;
        float w11 = fx * fy * inv8;

        return {
            p00.red   * w00 + p10.red   * w10 + p01.red   * w01 + p11.red   * w11,
            p00.green * w00 + p10.green * w10 + p01.green * w01 + p11.green * w11,
            p00.blue  * w00 + p10.blue  * w10 + p01.blue  * w01 + p11.blue  * w11,
            p00.alpha * w00 + p10.alpha * w10 + p01.alpha * w01 + p11.alpha * w11
        };
    }
}

/*==================================================================
 * 4. FULL PIXEL SYNTHESIS PIPELINE
 *==================================================================*/
inline FloatRGBA SynthesizePixel(const DreamyRenderParams& rp, A_long px, A_long py) {
    const float w = (float)rp.width;
    const float h = (float)rp.height;
    float u = ((float)px + 0.5f) / w;
    float v = ((float)py + 0.5f) / h;

    /*--------------------------------------------------------------
     * STAGE A: CRT Glass Curvature (Barrel Distortion)
     *--------------------------------------------------------------*/
    float vignette = 1.0f;
    if (rp.crt_enable && rp.crt_curvature > 0.001f) {
        float cu = u * 2.0f - 1.0f;
        float cv = v * 2.0f - 1.0f;
        float r2 = cu * cu + cv * cv;
        float k  = rp.crt_curvature * 0.22f;
        cu *= (1.0f + k * r2);
        cv *= (1.0f + k * r2);
        u = cu * 0.5f + 0.5f;
        v = cv * 0.5f + 0.5f;

        /* Soft CRT corner bezel darkening */
        float edge = std::max(std::fabs(cu), std::fabs(cv));
        if (edge > 1.0f) {
            return { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        vignette = 1.0f - SmoothStep(0.75f, 1.05f, std::sqrt(r2) * 0.75f) * (rp.crt_curvature * 0.65f);
    }

    float sample_x = u * w;
    float sample_y = v * h;

    /*--------------------------------------------------------------
     * STAGE B: VHS Tape Timebase Wobble & Tracking Static Bar
     *--------------------------------------------------------------*/
    float tracking_static_add = 0.0f;
    if (rp.vhs_enable) {
        float eff_wobble = rp.vhs_tape_wobble * (1.0f + rp.audio_bass * 1.35f);
        if (eff_wobble > 0.001f) {
            /* 60Hz mains hum wave + high-frequency head switching jitter */
            float hum = std::sin(v * 10.0f + rp.time_sec * 9.0f) * 3.5f;
            float flutter = std::sin(v * 75.0f - rp.time_sec * 37.0f) * 1.2f;
            float line_noise = (Hash21(std::floor(sample_y), rp.time_sec * 12.0f) - 0.5f) * 4.0f;
            sample_x += (hum + flutter + line_noise) * eff_wobble * rp.downsample_scale;
        }

        /* VHS Tracking Noise Bar (scrolls vertically and flares on bass hits) */
        float eff_track = rp.vhs_tracking_noise * (1.0f + rp.audio_bass * 0.8f);
        if (eff_track > 0.001f) {
            float bar_pos = std::fmod(0.88f - rp.time_sec * 0.12f, 1.0f);
            if (bar_pos < 0.0f) bar_pos += 1.0f;
            float dist = std::fabs(v - bar_pos);
            float bar_env = SmoothStep(0.08f * eff_track, 0.0f, dist);
            if (bar_env > 0.001f) {
                /* Strong horizontal tear inside the tracking band */
                float tear = (Hash21(std::floor(sample_y * 0.5f), rp.time_sec * 60.0f) - 0.5f) * 45.0f * bar_env * eff_track;
                sample_x += tear * rp.downsample_scale;
                float snow = Hash21(std::floor(sample_x * 0.5f), std::floor(sample_y) + rp.time_sec * 120.0f);
                if (snow > 0.45f) {
                    tracking_static_add = (snow - 0.45f) * 1.8f * bar_env * eff_track;
                }
            }
        }
    }

    /*--------------------------------------------------------------
     * STAGE C: Circuit Bending Horizontal Line-Hold Tear & Smear
     * (Reference Image 2 horizontal displacement & color smear)
     *--------------------------------------------------------------*/
    float bend_audio_mult = 1.0f + (rp.audio_high * 0.9f + rp.audio_bass * 0.6f) * rp.bend_audio_react;
    if (rp.bend_enable && rp.bend_smear_tear > 0.001f) {
        float eff_tear = rp.bend_smear_tear * bend_audio_mult;
        /* Quantized vertical band hold error */
        float band_id = std::floor(v * 28.0f + std::sin(rp.time_sec * 4.0f) * 3.0f);
        float band_hash = Hash21(band_id, std::floor(rp.time_sec * 15.0f));
        if (band_hash > (1.0f - std::min(0.85f, eff_tear * 0.65f))) {
            float tear_offset = (band_hash - 0.5f) * 110.0f * eff_tear * rp.downsample_scale;
            /* Smooth saw wave inside torn scanline band */
            sample_x += tear_offset * std::sin(v * 140.0f + rp.time_sec * 20.0f);
        }
    }

    /*--------------------------------------------------------------
     * STAGE D: Frequency Modulation (FM) & Rutt-Etra Scanline Synth
     * (Reference Image 1 glowing 3D-displaced FM oscilloscope waves)
     *--------------------------------------------------------------*/
    FloatRGBA base_col = SampleWorldBilinear(rp, sample_x, sample_y);
    float local_luma = Luminance(base_col);
    float rutt_ribbon_glow = 0.0f;
    float fm_carrier_signal = 0.0f;

    if (rp.fm_enable) {
        float fm_audio = 1.0f + (rp.audio_bass * 1.1f + rp.audio_mid * 0.6f) * rp.fm_audio_react;

        /* 1. True Phase/Frequency Modulation:
         * Brightness modulates the phase of a high-frequency carrier wave */
        float carrier_hz = rp.fm_carrier_freq * 0.5f;
        float mod_index  = rp.fm_mod_depth * fm_audio * 18.0f;

        /* Sample slightly smoothed horizontal luma for organic FM phase derivative */
        float luma_left  = Luminance(SampleWorldBilinear(rp, sample_x - 3.0f * rp.downsample_scale, sample_y));
        float luma_right = Luminance(SampleWorldBilinear(rp, sample_x + 3.0f * rp.downsample_scale, sample_y));
        float smooth_luma = 0.25f * luma_left + 0.5f * local_luma + 0.25f * luma_right;

        /* Horizontal FM displacement + carrier harmonic texture */
        float fm_phase = u * carrier_hz * (float)(2.0 * M_PI) + smooth_luma * mod_index + rp.time_sec * 6.0f;
        fm_carrier_signal = std::sin(fm_phase);

        float fm_x_shift = fm_carrier_signal * (rp.fm_mod_depth * 14.0f * fm_audio) * rp.downsample_scale;
        sample_x += fm_x_shift;

        /* 2. Rutt-Etra 3D Scanline Z-Elevation & Glowing Oscilloscope Ribbons:
         * In a Rutt-Etra video synthesizer, each horizontal scanline is deflected
         * vertically (Y) proportional to image luminance (Z) + FM wave modulation. */
        float z_elev = rp.fm_rutt_etra_z * fm_audio;
        if (z_elev > 0.001f || rp.fm_scanline_density > 0.001f) {
            /* Vertical deflection of sampling coordinate by luminance + subtle FM ripple */
            float y_deflect = (smooth_luma * z_elev * 55.0f + fm_carrier_signal * z_elev * 4.5f) * rp.downsample_scale;
            sample_y += y_deflect;

            /* Re-sample at the FM + Rutt-Etra deflected coordinates */
            base_col = SampleWorldBilinear(rp, sample_x, sample_y);
            local_luma = Luminance(base_col);

            /* Generate the iconic Reference Image 1 glowing horizontal oscilloscope contour lines */
            if (rp.fm_scanline_density > 1.0f) {
                float num_lines = rp.fm_scanline_density;
                /* Deformed scanline coordinate combines vertical position, luminance elevation, and FM wave */
                float deformed_v = v * num_lines
                                 - (local_luma * z_elev * (num_lines * 0.16f))
                                 + (fm_carrier_signal * rp.fm_mod_depth * 0.45f * fm_audio);

                /* Distance to nearest scanline center (0.0 at center, 0.5 at midpoint between lines) */
                float frac_line = deformed_v - std::floor(deformed_v);
                float dist_to_line = std::fabs(frac_line - 0.5f) * 2.0f; /* 0 at center, 1 at boundary */

                /* Beam thickness & phosphor bloom around each Rutt-Etra line */
                float beam_width = Clamp01(rp.fm_line_thickness);
                float core_beam  = SmoothStep(beam_width, beam_width * 0.15f, dist_to_line);

                /* High-frequency dotted/stippled FM beam modulation along the line (like Ref Image 1) */
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
        float eff_bleed = rp.vhs_chroma_bleed * (1.0f + rp.audio_mid * 0.8f);
        if (eff_bleed > 0.001f) {
            float shift_px = eff_bleed * 18.0f * rp.downsample_scale;
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
        float eff_bloom = rp.vhs_dream_bloom * (1.0f + rp.audio_mid * 0.9f + rp.audio_bass * 0.5f);
        if (eff_bloom > 0.001f) {
            float rad1 = (6.0f + eff_bloom * 12.0f) * rp.downsample_scale;
            float rad2 = (14.0f + eff_bloom * 26.0f) * rp.downsample_scale;

            /* 8-tap dreamy star/halo kernel around current sample */
            FloatRGBA b1 = SampleWorldBilinear(rp, sample_x + rad1, sample_y);
            FloatRGBA b2 = SampleWorldBilinear(rp, sample_x - rad1, sample_y);
            FloatRGBA b3 = SampleWorldBilinear(rp, sample_x, sample_y + rad1);
            FloatRGBA b4 = SampleWorldBilinear(rp, sample_x, sample_y - rad1);
            FloatRGBA b5 = SampleWorldBilinear(rp, sample_x + rad2 * 0.7f, sample_y + rad2 * 0.7f);
            FloatRGBA b6 = SampleWorldBilinear(rp, sample_x - rad2 * 0.7f, sample_y + rad2 * 0.7f);
            FloatRGBA b7 = SampleWorldBilinear(rp, sample_x + rad2 * 0.7f, sample_y - rad2 * 0.7f);
            FloatRGBA b8 = SampleWorldBilinear(rp, sample_x - rad2 * 0.7f, sample_y - rad2 * 0.7f);

            FloatRGBA avg_halo = {
                (b1.r + b2.r + b3.r + b4.r) * 0.15f + (b5.r + b6.r + b7.r + b8.r) * 0.10f,
                (b1.g + b2.g + b3.g + b4.g) * 0.15f + (b5.g + b6.g + b7.g + b8.g) * 0.10f,
                (b1.b + b2.b + b3.b + b4.b) * 0.15f + (b5.b + b6.b + b7.b + b8.b) * 0.10f,
                1.0f
            };

            /* Highlight knee extraction + dreamy warm/magenta halation tint */
            float halo_luma = Luminance(avg_halo);
            float highlight_boost = SmoothStep(0.18f, 0.90f, halo_luma) * eff_bloom * 0.85f;

            /* Screen-like soft additive blend */
            base_col.r = 1.0f - (1.0f - base_col.r) * (1.0f - Clamp01(avg_halo.r * highlight_boost * 1.15f));
            base_col.g = 1.0f - (1.0f - base_col.g) * (1.0f - Clamp01(avg_halo.g * highlight_boost * 0.95f));
            base_col.b = 1.0f - (1.0f - base_col.b) * (1.0f - Clamp01(avg_halo.b * highlight_boost * 1.20f));
        }
    }

    /*--------------------------------------------------------------
     * STAGE F: Circuit Bending (Analog Wavefolding & DAC Pin XOR)
     * (Reference Image 2 psychedelic color wrapping & glitch)
     *--------------------------------------------------------------*/
    if (rp.bend_enable) {
        /* 1. Cross-channel RGB feedback matrix rotation */
        if (rp.bend_feedback_shift > 0.001f) {
            float fb = rp.bend_feedback_shift * bend_audio_mult;
            float angle = fb * (float)M_PI * 1.5f;
            float ca = std::cos(angle);
            float sa = std::sin(angle);
            float nr = base_col.r * ca - base_col.g * sa * 0.6f + base_col.b * sa * 0.6f;
            float ng = base_col.g * ca + base_col.r * sa * 0.6f - base_col.b * sa * 0.6f;
            float nb = base_col.b * ca - base_col.r * sa * 0.6f + base_col.g * sa * 0.6f;
            base_col.r = Clamp01(std::fabs(nr));
            base_col.g = Clamp01(std::fabs(ng));
            base_col.b = Clamp01(std::fabs(nb));
        }

        /* 2. Analog Video Synth Phase Wavefolder / Solarization (Tachyons+ style) */
        if (rp.bend_phase_fold > 0.001f) {
            float fold_gain = 1.0f + rp.bend_phase_fold * bend_audio_mult * 4.5f;
            /* Different phase offsets per channel create psychedelic rainbow contours */
            base_col.r = 0.5f + 0.5f * std::sin((base_col.r * fold_gain - 0.25f) * (float)M_PI);
            base_col.g = 0.5f + 0.5f * std::sin((base_col.g * fold_gain * 1.12f - 0.10f) * (float)M_PI);
            base_col.b = 0.5f + 0.5f * std::sin((base_col.b * fold_gain * 1.25f + 0.15f) * (float)M_PI);
        }

        /* 3. Digital DAC Pin Short / Bitwise XOR Quantization */
        if (rp.bend_xor_crush > 0.001f) {
            float crush_amt = Clamp01(rp.bend_xor_crush * bend_audio_mult);
            uint8_t r8 = (uint8_t)(Clamp01(base_col.r) * 255.0f);
            uint8_t g8 = (uint8_t)(Clamp01(base_col.g) * 255.0f);
            uint8_t b8 = (uint8_t)(Clamp01(base_col.b) * 255.0f);

            /* Emulate shorted data pins on an 8-bit video ADC/DAC */
            uint8_t mask = (uint8_t)((int)(crush_amt * 170.0f) & 0xFC);
            uint8_t xr = (r8 ^ ((g8 >> 1) & mask));
            uint8_t xg = (g8 ^ ((b8 >> 1) & mask));
            uint8_t xb = (b8 ^ ((r8 >> 1) & mask));

            base_col.r = Lerp(base_col.r, xr / 255.0f, crush_amt * 0.85f);
            base_col.g = Lerp(base_col.g, xg / 255.0f, crush_amt * 0.85f);
            base_col.b = Lerp(base_col.b, xb / 255.0f, crush_amt * 0.85f);
        }
    }

    /*--------------------------------------------------------------
     * STAGE G: 4-Stop Editable / Preset Gradient Colorizer
     *--------------------------------------------------------------*/
    if (rp.gradient_mix > 0.001f) {
        /* Map pixel luminance + FM wave harmonic + Rutt-Etra ribbon intensity through the 4-stop gradient */
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
     * (Reference Image 3 discrete R-G-B vertical phosphor stripes + scanlines)
     *--------------------------------------------------------------*/
    if (rp.crt_enable) {
        float pitch = std::max(1.5f, rp.crt_scanline_pitch * rp.downsample_scale);

        /* 1. Horizontal CRT Scanline Beam Profile */
        if (rp.crt_scanline_intensity > 0.001f) {
            float scan_phase = ((float)py / pitch) * (float)(2.0 * M_PI);
            /* Raised cosine beam profile with bright phosphor bloom compensation */
            float scan_wave = 0.5f + 0.5f * std::cos(scan_phase);
            /* Bright pixels bloom wider across scanline gaps on real CRTs */
            float pix_luma = Luminance(base_col);
            float bloom_comp = Lerp(scan_wave, std::sqrt(scan_wave), pix_luma * 0.65f);
            float scan_mult = Lerp(1.0f, bloom_comp * 1.15f, rp.crt_scanline_intensity);

            base_col.r *= scan_mult;
            base_col.g *= scan_mult;
            base_col.b *= scan_mult;
        }

        /* 2. RGB Phosphor Aperture Grille / Shadow Mask / Slot Mask */
        if (rp.crt_phosphor_mask != CRT_MASK_NONE && rp.crt_phosphor_strength > 0.001f) {
            float cell_w = std::max(1.0f, (pitch * 0.5f));
            int sub_x = (int)std::floor((float)px / cell_w);
            int sub_y = (int)std::floor((float)py / pitch);

            if (rp.crt_phosphor_mask == CRT_MASK_SHADOW_TRIAD) {
                /* Staggered delta triad offsets every other scanline by half a triad */
                sub_x += (sub_y & 1) ? 1 : 0;
            }

            int triad_idx = ((sub_x % 3) + 3) % 3;
            float mask_r = 0.18f, mask_g = 0.18f, mask_b = 0.18f;

            /* Discrete R, G, B phosphor cells with energy-preserving boost (like Reference Image 3) */
            if (triad_idx == 0) {
                mask_r = 1.48f; mask_g = 0.15f; mask_b = 0.12f;
            } else if (triad_idx == 1) {
                mask_r = 0.15f; mask_g = 1.48f; mask_b = 0.15f;
            } else {
                mask_r = 0.12f; mask_g = 0.15f; mask_b = 1.48f;
            }

            /* Slot mask adds dark horizontal slot bridges between vertical phosphor stripes */
            if (rp.crt_phosphor_mask == CRT_MASK_SLOT_MASK) {
                float slot_v = std::fmod((float)py + ((sub_x & 1) ? pitch * 0.5f : 0.0f), pitch) / pitch;
                if (slot_v > 0.82f) {
                    mask_r *= 0.45f; mask_g *= 0.45f; mask_b *= 0.45f;
                }
            }

            float str = rp.crt_phosphor_strength;
            base_col.r *= Lerp(1.0f, mask_r, str);
            base_col.g *= Lerp(1.0f, mask_g, str);
            base_col.b *= Lerp(1.0f, mask_b, str);
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

} /* namespace DreamyDSP */

#endif /* DREAMY_DSP_H */
