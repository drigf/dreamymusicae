/*******************************************************************/
/*                                                                 */
/*  DreamyMusic.h                                                  */
/*  Audio-Reactive Dreamy VHS, FM Rutt-Etra, Circuit Bending,      */
/*  Gradient Synthesizer & CRT Scanline After Effects Plugin       */
/*                                                                 */
/*******************************************************************/

#ifndef DREAMYMUSIC_H
#define DREAMYMUSIC_H

#include "AE_Effect.h"
#include "AE_Macros.h"

#define MAJOR_VERSION       1
#define MINOR_VERSION       0
#define BUG_VERSION         0
#define STAGE_VERSION       PF_Stage_DEVELOP
#define BUILD_VERSION       1

#define PLUGIN_NAME         "Dreamy Music VHS"
#define PLUGIN_MATCH_NAME   "ADBE DreamyMusicVHS"
#define PLUGIN_CATEGORY     "Stylize"

/* Parameter Enumeration (Checkout Index) */
enum {
    DREAMY_INPUT = 0,

    /* 1. Audio & Music Reactivity */
    PARAM_TOPIC_AUDIO_START,
    PARAM_AUDIO_LAYER,              /* Layer picker for direct audio analysis */
    PARAM_AUDIO_SLIDER,             /* Keyframable slider (for Convert Audio to Keyframes / Expressions) */
    PARAM_AUDIO_GAIN,               /* Master Audio Reactivity Multiplier (0 - 500%) */
    PARAM_AUDIO_BASS_WEIGHT,        /* Sub/Bass reactivity weight (FM & VHS Tracking) */
    PARAM_AUDIO_MID_WEIGHT,         /* Midrange reactivity weight (Gradient & Bloom) */
    PARAM_AUDIO_HIGH_WEIGHT,        /* Treble/High reactivity weight (Circuit Bend & Glitch) */
    PARAM_AUDIO_SMOOTHING,          /* Temporal window smoothing (ms) */
    PARAM_TOPIC_AUDIO_END,

    /* 2. Gradient Color Palette */
    PARAM_TOPIC_GRADIENT_START,
    PARAM_GRADIENT_ENABLE,          /* Enable / Disable Gradient Colorizer (keep original colors when off) */
    PARAM_GRADIENT_MODE,            /* Popup: Custom 4-Stop, Sunset Dream, Neon Oscilloscope, Acid Circuit, Cyberpunk VHS, Thermal CRT, Pastel Vaporwave, Off */
    PARAM_COLOR_SHADOW,             /* Stop 1: Deep Shadows */
    PARAM_COLOR_MID_LOW,            /* Stop 2: Low Midtones */
    PARAM_COLOR_MID_HIGH,           /* Stop 3: High Midtones */
    PARAM_COLOR_HIGHLIGHT,          /* Stop 4: Bright Highlights */
    PARAM_GRADIENT_MIX,             /* Blend with original layer color (0 - 100%) */
    PARAM_GRADIENT_CYCLE,           /* Color phase shift / cycle speed */
    PARAM_GRADIENT_AUDIO_SHIFT,     /* Audio-driven color phase cycling */
    PARAM_TOPIC_GRADIENT_END,

    /* 3. Frequency Modulation (FM) & Rutt-Etra Oscilloscope */
    PARAM_TOPIC_FM_START,
    PARAM_FM_ENABLE,                /* Enable FM / Rutt-Etra Synthesis */
    PARAM_FM_CARRIER_FREQ,          /* Carrier frequency (horizontal wave density) */
    PARAM_FM_MOD_DEPTH,             /* Luma-to-Phase FM modulation depth */
    PARAM_FM_RUTT_ETRA_Z,           /* 3D Scanline vertical elevation (Rutt-Etra Z-extrusion) */
    PARAM_FM_SCANLINE_DENSITY,      /* Number of glowing Rutt-Etra horizontal scan ribbons */
    PARAM_FM_LINE_THICKNESS,        /* Ribbon beam thickness / glow falloff */
    PARAM_FM_AUDIO_REACT,           /* How strongly music drives FM & Rutt-Etra elevation */
    PARAM_TOPIC_FM_END,

    /* 4. Circuit Bending (Analog Video Synth Glitch) */
    PARAM_TOPIC_BEND_START,
    PARAM_BEND_ENABLE,              /* Enable Circuit Bending */
    PARAM_BEND_PHASE_FOLD,          /* Analog wavefolder / RGB solarization wraps */
    PARAM_BEND_XOR_CRUSH,           /* Digital DAC pin short / bitwise XOR quantization */
    PARAM_BEND_SMEAR_TEAR,          /* Horizontal line hold tear & analog color smear */
    PARAM_BEND_FEEDBACK_SHIFT,      /* Cross-channel RGB matrix bend */
    PARAM_BEND_AUDIO_REACT,         /* Music-triggered circuit bend spikes */
    PARAM_TOPIC_BEND_END,

    /* 5. Dreamy VHS & Tape Aesthetics */
    PARAM_TOPIC_VHS_START,
    PARAM_VHS_ENABLE,               /* Enable Dreamy VHS & Bloom */
    PARAM_VHS_DREAM_BLOOM,          /* Soft halation & dreamy highlight glow */
    PARAM_VHS_CHROMA_BLEED,         /* YIQ/YUV chroma delay & horizontal color smear */
    PARAM_VHS_TAPE_WOBBLE,          /* Analog 60Hz timebase jitter & tape flutter */
    PARAM_VHS_TRACKING_NOISE,       /* Bottom/floating VHS tape tracking static bar */
    PARAM_VHS_GRAIN,                /* Analog magnetic tape noise */
    PARAM_TOPIC_VHS_END,

    /* 6. CRT Scanlines & Trinitron Phosphor Mask */
    PARAM_TOPIC_CRT_START,
    PARAM_CRT_ENABLE,               /* Enable CRT Display Emulation */
    PARAM_CRT_SCANLINE_PITCH,       /* Scanline height in pixels (e.g. 2.0 - 12.0 px) */
    PARAM_CRT_SCANLINE_INTENSITY,   /* Scanline darkness / contrast */
    PARAM_CRT_PHOSPHOR_MASK,        /* Popup: None, Trinitron Aperture Grille, Shadow Mask Dot Triad, Slot Mask */
    PARAM_CRT_PHOSPHOR_STRENGTH,    /* RGB Subpixel grille visibility */
    PARAM_CRT_CURVATURE,            /* Subtle CRT glass barrel distortion & vignette */
    PARAM_TOPIC_CRT_END,

    /* 7. Dreamy Echo Trail (Windows Cursor Trail Style Temporal/Spatial Ghosting) */
    PARAM_TOPIC_ECHO_START,
    PARAM_ECHO_ENABLE,              /* Enable / Disable Echo Trail */
    PARAM_ECHO_COUNT,               /* Number of ghost copies in the trail (1 - 12) */
    PARAM_ECHO_DELAY,               /* Time delay between trail copies in frames (0.2 - 8.0 frames) */
    PARAM_ECHO_OFFSET,              /* Spatial cursor-trail offset / drift distance (px) */
    PARAM_ECHO_OPACITY,             /* Trail transparency / opacity (0 - 100%) */
    PARAM_ECHO_DECAY,               /* How quickly each successive ghost copy fades out (0 - 100%) */
    PARAM_ECHO_GLOW,                /* Customizable glow intensity around the echo trail (0 - 200%) */
    PARAM_ECHO_GLOW_COLOR,          /* Custom glow tint color for the echo trail */
    PARAM_TOPIC_ECHO_END,

    DREAMY_NUM_PARAMS
};

/* Disk IDs (never reorder once shipped) */
enum {
    ID_TOPIC_AUDIO_START = 100,
    ID_AUDIO_LAYER,
    ID_AUDIO_SLIDER,
    ID_AUDIO_GAIN,
    ID_AUDIO_BASS_WEIGHT,
    ID_AUDIO_MID_WEIGHT,
    ID_AUDIO_HIGH_WEIGHT,
    ID_AUDIO_SMOOTHING,
    ID_TOPIC_AUDIO_END,

    ID_TOPIC_GRADIENT_START = 200,
    ID_GRADIENT_MODE,
    ID_COLOR_SHADOW,
    ID_COLOR_MID_LOW,
    ID_COLOR_MID_HIGH,
    ID_COLOR_HIGHLIGHT,
    ID_GRADIENT_MIX,
    ID_GRADIENT_CYCLE,
    ID_GRADIENT_AUDIO_SHIFT,
    ID_TOPIC_GRADIENT_END,
    ID_GRADIENT_ENABLE = 220,

    ID_TOPIC_FM_START = 300,
    ID_FM_ENABLE,
    ID_FM_CARRIER_FREQ,
    ID_FM_MOD_DEPTH,
    ID_FM_RUTT_ETRA_Z,
    ID_FM_SCANLINE_DENSITY,
    ID_FM_LINE_THICKNESS,
    ID_FM_AUDIO_REACT,
    ID_TOPIC_FM_END,

    ID_TOPIC_BEND_START = 400,
    ID_BEND_ENABLE,
    ID_BEND_PHASE_FOLD,
    ID_BEND_XOR_CRUSH,
    ID_BEND_SMEAR_TEAR,
    ID_BEND_FEEDBACK_SHIFT,
    ID_BEND_AUDIO_REACT,
    ID_TOPIC_BEND_END,

    ID_TOPIC_VHS_START = 500,
    ID_VHS_ENABLE,
    ID_VHS_DREAM_BLOOM,
    ID_VHS_CHROMA_BLEED,
    ID_VHS_TAPE_WOBBLE,
    ID_VHS_TRACKING_NOISE,
    ID_VHS_GRAIN,
    ID_TOPIC_VHS_END,

    ID_TOPIC_CRT_START = 600,
    ID_CRT_ENABLE,
    ID_CRT_SCANLINE_PITCH,
    ID_CRT_SCANLINE_INTENSITY,
    ID_CRT_PHOSPHOR_MASK,
    ID_CRT_PHOSPHOR_STRENGTH,
    ID_CRT_CURVATURE,
    ID_TOPIC_CRT_END,

    ID_TOPIC_ECHO_START = 700,
    ID_ECHO_ENABLE,
    ID_ECHO_COUNT,
    ID_ECHO_DELAY,
    ID_ECHO_OFFSET,
    ID_ECHO_OPACITY,
    ID_ECHO_DECAY,
    ID_ECHO_GLOW,
    ID_ECHO_GLOW_COLOR,
    ID_TOPIC_ECHO_END
};

/* Gradient Preset Modes (1-based popup index in AE) */
enum {
    GRAD_MODE_CUSTOM = 1,
    GRAD_MODE_SUNSET_DREAM,         /* Deep black/purple -> magenta -> neon orange -> pastel cyan/white */
    GRAD_MODE_ACID_CIRCUIT,         /* Deep indigo -> electric green -> acid yellow -> hot magenta */
    GRAD_MODE_NEON_OSCILLOSCOPE,    /* Amber gold -> hot pink -> electric violet -> laser cyan */
    GRAD_MODE_CYBERPUNK_VHS,        /* Midnight blue -> neon magenta -> cyan -> warm cream */
    GRAD_MODE_THERMAL_CRT,          /* Void black -> crimson -> solar gold -> incandescent white */
    GRAD_MODE_PASTEL_VAPORWAVE,     /* Twilight plum -> dreamy rose -> mint aqua -> soft lavender */
    GRAD_MODE_OFF                   /* Off: Preserve 100% original footage colors */
};

/* CRT Phosphor Mask Modes (1-based popup index in AE) */
enum {
    CRT_MASK_NONE = 1,
    CRT_MASK_TRINITRON_GRILLE,      /* Vertical R-G-B aperture grille stripes */
    CRT_MASK_SHADOW_TRIAD,          /* Staggered delta RGB phosphor triad */
    CRT_MASK_SLOT_MASK              /* Arcade CRT staggered vertical slot mask */
};

/* Normalized RGB float helper */
typedef struct {
    float r, g, b, a;
} FloatRGBA;

/* Evaluated Frame Parameters & Audio Analysis State */
typedef struct {
    /* Dimensions & Time */
    A_long      width;
    A_long      height;
    float       time_sec;
    A_long      frame_num;
    float       downsample_scale;

    /* Evaluated Audio Reactivity Envelopes (0.0 -> 2.0+) */
    float       audio_bass;
    float       audio_mid;
    float       audio_high;
    float       audio_overall;

    /* Raw Audio Checkout Diagnostics (for detailed log) */
    PF_Err      audio_checkout_err;
    A_long      audio_num_samples;
    float       audio_raw_bass;
    float       audio_raw_mid;
    float       audio_raw_high;
    float       audio_peak_sample;

    /* Gradient Settings */
    A_Boolean   gradient_enable;
    A_long      gradient_mode;
    FloatRGBA   stops[4];           /* 4-stop active palette */
    float       gradient_mix;       /* 0.0 - 1.0 */
    float       gradient_phase;     /* Combined manual + audio phase offset */

    /* FM & Rutt-Etra Settings */
    A_Boolean   fm_enable;
    float       fm_carrier_freq;
    float       fm_mod_depth;
    float       fm_rutt_etra_z;
    float       fm_scanline_density;
    float       fm_line_thickness;
    float       fm_audio_react;

    /* Circuit Bending Settings */
    A_Boolean   bend_enable;
    float       bend_phase_fold;
    float       bend_xor_crush;
    float       bend_smear_tear;
    float       bend_feedback_shift;
    float       bend_audio_react;

    /* Dreamy VHS Settings */
    A_Boolean   vhs_enable;
    float       vhs_dream_bloom;
    float       vhs_chroma_bleed;
    float       vhs_tape_wobble;
    float       vhs_tracking_noise;
    float       vhs_grain;

    /* CRT Scanlines & Phosphor Mask */
    A_Boolean   crt_enable;
    float       crt_scanline_pitch;
    float       crt_scanline_intensity;
    A_long      crt_phosphor_mask;
    float       crt_phosphor_strength;
    float       crt_curvature;

    /* Echo Trail (Temporal + Spatial Ghosting & Glow) */
    A_Boolean   echo_enable;
    A_long      echo_count;             /* 1 .. 12 copies */
    float       echo_delay_frames;      /* Frame step between copies */
    float       echo_offset_px;         /* Spatial cursor-trail offset distance */
    float       echo_opacity;           /* 0.0 - 1.0 starting trail opacity (user transparency control) */
    float       echo_decay;             /* 0.0 - 1.0 per-step transparency falloff */
    float       echo_glow;              /* 0.0 - 2.0 customizable trail glow intensity */
    FloatRGBA   echo_glow_color;        /* Custom RGB glow tint for the trail */
    A_long      echo_valid_worlds;      /* Number of checked-out historical frames */
    const PF_EffectWorld *echo_worlds[12]; /* Historical input layer buffers */

    /* Precomputed Frame-Invariant Optimization Cache */
    float       inv_width;
    float       inv_height;
    float       fm_audio_mult;
    float       fm_carrier_rad;
    float       fm_mod_index;
    float       fm_x_shift_scale;
    float       fm_z_elev;
    float       bend_audio_mult;
    float       bend_fb_cos;
    float       bend_fb_sin;
    float       bend_fold_gain;
    float       bend_crush_amt;
    uint8_t     bend_xor_mask;
    float       vhs_eff_wobble;
    float       vhs_eff_track;
    float       vhs_bar_pos;
    float       vhs_eff_bleed_px;
    float       vhs_eff_bloom;
    float       vhs_bloom_rad1;
    float       vhs_bloom_rad2;
    float       crt_pitch;
    float       crt_inv_pitch;
    float       crt_cell_w;
    float       crt_inv_cell_w;
    A_long      echo_active_count;
    float       echo_glow_strength;
    float       echo_trail_mix;
    float       echo_step_alpha[12];
    float       echo_step_dx[12];
    float       echo_step_dy[12];
    float       echo_step_glow_rad[12];
    A_Boolean   echo_step_deep[12];
    FloatRGBA   gradient_lut[1024];

    /* Pointer to input buffer for spatial sampling */
    const PF_EffectWorld *input_world;
    PF_EffectWorld       *output_world;
    A_Boolean             is_16bit;
} DreamyRenderParams;

#ifdef __cplusplus
extern "C" {
#endif

DllExport PF_Err
EffectMain(
    PF_Cmd          cmd,
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output,
    void            *extra);

#ifdef __cplusplus
}
#endif

#endif /* DREAMYMUSIC_H */
