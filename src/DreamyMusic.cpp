/*******************************************************************/
/*                                                                 */
/*  DreamyMusic.cpp                                                */
/*  Main Adobe After Effects Plugin Implementation                 */
/*                                                                 */
/*******************************************************************/

#define DREAMY_LOG_ADD_PARAM 1
static void DreamyLog(const char* fmt, ...);
#include "DreamyMusic.h"
#include "DreamyDSP.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <ctime>

#define DREAMY_LOG_PATH "/tmp/dreamymusic.log"

/*------------------------------------------------------------------
 * Diagnostic File Logger (/tmp/dreamymusic.log)
 *------------------------------------------------------------------*/
static void DreamyLog(const char* fmt, ...) {
    FILE* fp = std::fopen(DREAMY_LOG_PATH, "a");
    if (!fp) return;

    std::time_t now = std::time(nullptr);
    struct std::tm* t = std::localtime(&now);
    if (t) {
        std::fprintf(fp, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    }

    va_list args;
    va_start(args, fmt);
    std::vfprintf(fp, fmt, args);
    va_end(args);

    std::fprintf(fp, "\n");
    std::fflush(fp);
    std::fclose(fp);
}

static const char* CmdToString(PF_Cmd cmd) {
    switch (cmd) {
        case PF_Cmd_ABOUT:              return "PF_Cmd_ABOUT";
        case PF_Cmd_GLOBAL_SETUP:       return "PF_Cmd_GLOBAL_SETUP";
        case PF_Cmd_GLOBAL_SETDOWN:     return "PF_Cmd_GLOBAL_SETDOWN";
        case PF_Cmd_PARAMS_SETUP:       return "PF_Cmd_PARAMS_SETUP";
        case PF_Cmd_SEQUENCE_SETUP:     return "PF_Cmd_SEQUENCE_SETUP";
        case PF_Cmd_SEQUENCE_RESETUP:   return "PF_Cmd_SEQUENCE_RESETUP";
        case PF_Cmd_SEQUENCE_FLATTEN:   return "PF_Cmd_SEQUENCE_FLATTEN";
        case PF_Cmd_SEQUENCE_SETDOWN:   return "PF_Cmd_SEQUENCE_SETDOWN";
        case PF_Cmd_FRAME_SETUP:        return "PF_Cmd_FRAME_SETUP";
        case PF_Cmd_RENDER:             return "PF_Cmd_RENDER";
        case PF_Cmd_FRAME_SETDOWN:      return "PF_Cmd_FRAME_SETDOWN";
        case PF_Cmd_USER_CHANGED_PARAM: return "PF_Cmd_USER_CHANGED_PARAM";
        case PF_Cmd_UPDATE_PARAMS_UI:   return "PF_Cmd_UPDATE_PARAMS_UI";
        case PF_Cmd_SMART_PRE_RENDER:   return "PF_Cmd_SMART_PRE_RENDER";
        case PF_Cmd_SMART_RENDER:       return "PF_Cmd_SMART_RENDER";
        default:                        return "PF_Cmd_OTHER";
    }
}

/*------------------------------------------------------------------
 * PluginDataEntryFunction2: Modern AE 2024-2026 Entry Verification
 *------------------------------------------------------------------*/
extern "C" DllExport PF_Err
PluginDataEntryFunction2(
    void                *inPtr,
    PF_PluginDataCB2    inPluginDataCallBackPtr,
    void                *inSPBasicSuitePtr,
    const char          *inHostName,
    const char          *inHostVersion)
{
    DreamyLog("PluginDataEntryFunction2 called: host='%s' version='%s'",
              inHostName ? inHostName : "null",
              inHostVersion ? inHostVersion : "null");

    PF_Err result = PF_Err_INVALID_INDEX;
    if (inPluginDataCallBackPtr) {
        result = inPluginDataCallBackPtr(
            inPtr,
            reinterpret_cast<const A_u_char*>(PLUGIN_NAME),
            reinterpret_cast<const A_u_char*>(PLUGIN_MATCH_NAME),
            reinterpret_cast<const A_u_char*>(PLUGIN_CATEGORY),
            reinterpret_cast<const A_u_char*>("EffectMain"),
            'eFKT',
            PF_PLUG_IN_VERSION,
            PF_PLUG_IN_SUBVERS,
            8,
            reinterpret_cast<const A_u_char*>(""));
    }
    DreamyLog("PluginDataEntryFunction2 result=%d", (int)result);
    return result;
}

/*------------------------------------------------------------------
 * PF_Cmd_ABOUT: Displays plugin info in After Effects About dialog
 *------------------------------------------------------------------*/
static PF_Err
About(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    std::snprintf(
        out_data->return_msg,
        sizeof(out_data->return_msg),
        "%s v%d.%d\r"
        "Audio-Reactive Dreamy VHS, FM Rutt-Etra Oscilloscope,\r"
        "Circuit Bending, 4-Stop Gradient Synth & CRT Scanlines.",
        PLUGIN_NAME,
        MAJOR_VERSION,
        MINOR_VERSION);
    return PF_Err_NONE;
}

/*------------------------------------------------------------------
 * PF_Cmd_GLOBAL_SETUP: Advertise plugin capabilities & flags to AE
 *------------------------------------------------------------------*/
static PF_Err
GlobalSetup(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    out_data->my_version = PF_VERSION(
        MAJOR_VERSION,
        MINOR_VERSION,
        BUG_VERSION,
        STAGE_VERSION,
        BUILD_VERSION);

    out_data->out_flags =
        PF_OutFlag_NON_PARAM_VARY           |
        PF_OutFlag_PIX_INDEPENDENT          |
        PF_OutFlag_I_USE_AUDIO              |
        PF_OutFlag_DEEP_COLOR_AWARE         |
        PF_OutFlag_SEND_UPDATE_PARAMS_UI;

    out_data->out_flags2 =
        PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG |
        PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

    DreamyLog("GlobalSetup complete: my_version=0x%x (%d), out_flags=0x%x, out_flags2=0x%x, sizeof(PF_InData)=%zu, sizeof(PF_OutData)=%zu, sizeof(PF_ParamDef)=%zu",
              (unsigned int)out_data->my_version,
              (int)out_data->my_version,
              (unsigned int)out_data->out_flags,
              (unsigned int)out_data->out_flags2,
              sizeof(PF_InData),
              sizeof(PF_OutData),
              sizeof(PF_ParamDef));

    return PF_Err_NONE;
}

/*------------------------------------------------------------------
 * PF_Cmd_PARAMS_SETUP: Register all Effect Controls groups & params
 *------------------------------------------------------------------*/
static PF_Err
ParamsSetup(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    PF_ParamDef def;

    /*==============================================================
     * 1. AUDIO & MUSIC REACTIVITY
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Music & Audio Reactivity", ID_TOPIC_AUDIO_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Audio Source Layer", PF_LayerDefault_MYSELF, ID_AUDIO_LAYER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Keyframe Slider",
        0.0, 200.0, 0.0, 50.0, 0.0,
        2, PF_ValueDisplayFlag_NONE, 0,
        ID_AUDIO_SLIDER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Master Audio Reactivity",
        0.0, 500.0, 0.0, 250.0, 125.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_GAIN);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Bass Weight (FM & VHS)",
        0.0, 300.0, 0.0, 200.0, 130.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_BASS_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Mid Weight (Bloom & Color)",
        0.0, 300.0, 0.0, 200.0, 100.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_MID_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "High Weight (Circuit Bend)",
        0.0, 300.0, 0.0, 200.0, 120.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_HIGH_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Window Smooth (ms)",
        15.0, 350.0, 25.0, 200.0, 75.0,
        0, PF_ValueDisplayFlag_NONE, 0,
        ID_AUDIO_SMOOTHING);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_AUDIO_END);

    /*==============================================================
     * 2. GRADIENT COLOR PALETTE (PRESETS & CUSTOM 4-STOP)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Dreamy Gradient Palette", ID_TOPIC_GRADIENT_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUPX(
        "Gradient Palette Mode",
        7,
        GRAD_MODE_SUNSET_DREAM,
        "Custom 4-Stop Gradient|"
        "Sunset Dream (Ref 1 FM)|"
        "Acid Circuit Synth (Ref 2)|"
        "Neon Oscilloscope|"
        "Cyberpunk VHS|"
        "Thermal Phosphor CRT|"
        "Pastel Vaporwave",
        PF_ParamFlag_SUPERVISE,
        ID_GRADIENT_MODE);

    /* Default custom stops match Sunset Dream so switching to Custom starts from a gorgeous palette */
    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Shadow Color (Stop 1)", 20, 5, 36, ID_COLOR_SHADOW);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Low Midtone (Stop 2)", 224, 38, 166, ID_COLOR_MID_LOW);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("High Midtone (Stop 3)", 255, 143, 26, ID_COLOR_MID_HIGH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Highlight Color (Stop 4)", 184, 230, 255, ID_COLOR_HIGHLIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Gradient Color Mix",
        0.0, 100.0, 0.0, 100.0, 85.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_GRADIENT_MIX);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Gradient Phase Offset",
        -500.0, 500.0, -100.0, 100.0, 0.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_GRADIENT_CYCLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Phase Cycling",
        0.0, 200.0, 0.0, 100.0, 35.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_GRADIENT_AUDIO_SHIFT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_GRADIENT_END);

    /*==============================================================
     * 3. FREQUENCY MODULATION (FM) & RUTT-ETRA OSCILLOSCOPE
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("FM & Rutt-Etra Synthesis", ID_TOPIC_FM_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable FM & Rutt-Etra", TRUE, 0, ID_FM_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Carrier Frequency",
        1.0, 400.0, 10.0, 200.0, 85.0,
        1, PF_ValueDisplayFlag_NONE, 0,
        ID_FM_CARRIER_FREQ);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Modulation Depth",
        0.0, 100.0, 0.0, 100.0, 25.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_MOD_DEPTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Rutt-Etra 3D Elevation",
        0.0, 200.0, 0.0, 100.0, 35.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_RUTT_ETRA_Z);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Oscilloscope Line Count",
        0.0, 400.0, 0.0, 220.0, 90.0,
        0, PF_ValueDisplayFlag_NONE, 0,
        ID_FM_SCANLINE_DENSITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Beam Ribbon Thickness",
        5.0, 100.0, 15.0, 95.0, 62.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_LINE_THICKNESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Audio Reactivity",
        0.0, 300.0, 0.0, 200.0, 90.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_AUDIO_REACT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_FM_END);

    /*==============================================================
     * 4. CIRCUIT BENDING (ANALOG VIDEO SYNTH GLITCH)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Circuit Bending", ID_TOPIC_BEND_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable Circuit Bending", TRUE, 0, ID_BEND_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Analog Phase Wavefolder",
        0.0, 100.0, 0.0, 100.0, 18.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_PHASE_FOLD);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "DAC Pin Short (XOR Crush)",
        0.0, 100.0, 0.0, 100.0, 0.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_XOR_CRUSH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Horizontal Tear & Smear",
        0.0, 100.0, 0.0, 100.0, 15.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_SMEAR_TEAR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Chroma Matrix Bend",
        0.0, 100.0, 0.0, 100.0, 12.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_FEEDBACK_SHIFT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Bend Audio Reactivity",
        0.0, 300.0, 0.0, 200.0, 100.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_AUDIO_REACT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_BEND_END);

    /*==============================================================
     * 5. DREAMY VHS & TAPE AESTHETICS
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Dreamy VHS & Halation", ID_TOPIC_VHS_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable Dreamy VHS", TRUE, 0, ID_VHS_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Dreamy Halation Bloom",
        0.0, 150.0, 0.0, 100.0, 55.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_DREAM_BLOOM);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "VHS Chroma Bleed",
        0.0, 100.0, 0.0, 100.0, 42.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_CHROMA_BLEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Tape Timebase Wobble",
        0.0, 100.0, 0.0, 100.0, 24.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_TAPE_WOBBLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Tracking Static Bar",
        0.0, 100.0, 0.0, 100.0, 20.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_TRACKING_NOISE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Analog Tape Grain",
        0.0, 100.0, 0.0, 100.0, 18.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_GRAIN);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_VHS_END);

    /*==============================================================
     * 6. CRT SCANLINES & TRINITRON PHOSPHOR MASK
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("CRT Scanlines & Phosphor", ID_TOPIC_CRT_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable CRT Display", TRUE, 0, ID_CRT_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Scanline Pitch (px)",
        2.0, 24.0, 2.0, 12.0, 4.0,
        1, PF_ValueDisplayFlag_PIXEL, 0,
        ID_CRT_SCANLINE_PITCH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Scanline Intensity",
        0.0, 100.0, 0.0, 100.0, 45.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_CRT_SCANLINE_INTENSITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUPX(
        "RGB Phosphor Mask Type",
        4,
        CRT_MASK_TRINITRON_GRILLE,
        "None (Scanlines Only)|"
        "Trinitron Aperture Grille (Ref 3)|"
        "Shadow Mask Delta Triad|"
        "Arcade Slot Mask",
        0,
        ID_CRT_PHOSPHOR_MASK);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Phosphor Subpixel Strength",
        0.0, 100.0, 0.0, 100.0, 40.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_CRT_PHOSPHOR_STRENGTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "CRT Glass Curvature",
        0.0, 100.0, 0.0, 100.0, 15.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_CRT_CURVATURE);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_CRT_END);

    out_data->num_params = DREAMY_NUM_PARAMS;
    DreamyLog("ParamsSetup complete: registered num_params=%d (offset of out_data->num_params=%zu, offset of in_data->effect_ref=%zu)",
              (int)out_data->num_params,
              offsetof(PF_OutData, num_params),
              offsetof(PF_InData, effect_ref));
    return PF_Err_NONE;
}

/*------------------------------------------------------------------
 * Multi-Threaded Row Renderer Callback (8-bit and 16-bit Deep Color)
 *------------------------------------------------------------------*/
static PF_Err
RenderRowTask(
    void        *refconPV,
    A_long      thread_indexL,
    A_long      y,
    A_long      iterationsL)
{
    DreamyRenderParams* rp = reinterpret_cast<DreamyRenderParams*>(refconPV);
    if (!rp || !rp->output_world || !rp->output_world->data) {
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    char* dst_base = reinterpret_cast<char*>(rp->output_world->data);

    if (rp->is_16bit) {
        PF_Pixel16* out_row = reinterpret_cast<PF_Pixel16*>(dst_base + y * rp->output_world->rowbytes);
        for (A_long x = 0; x < rp->width; ++x) {
            FloatRGBA c = DreamyDSP::SynthesizePixel(*rp, x, y);
            out_row[x].red   = (A_u_short)(c.r * 32768.0f + 0.5f);
            out_row[x].green = (A_u_short)(c.g * 32768.0f + 0.5f);
            out_row[x].blue  = (A_u_short)(c.b * 32768.0f + 0.5f);
            out_row[x].alpha = (A_u_short)(c.a * 32768.0f + 0.5f);
        }
    } else {
        PF_Pixel8* out_row = reinterpret_cast<PF_Pixel8*>(dst_base + y * rp->output_world->rowbytes);
        for (A_long x = 0; x < rp->width; ++x) {
            FloatRGBA c = DreamyDSP::SynthesizePixel(*rp, x, y);
            out_row[x].red   = (A_u_char)(c.r * 255.0f + 0.5f);
            out_row[x].green = (A_u_char)(c.g * 255.0f + 0.5f);
            out_row[x].blue  = (A_u_char)(c.b * 255.0f + 0.5f);
            out_row[x].alpha = (A_u_char)(c.a * 255.0f + 0.5f);
        }
    }

    return PF_Err_NONE;
}

/*------------------------------------------------------------------
 * PF_Cmd_RENDER: Evaluate audio & parameters and synthesize frame
 *------------------------------------------------------------------*/
static PF_Err
Render(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    PF_Err err = PF_Err_NONE;
    PF_EffectWorld* input = &params[DREAMY_INPUT]->u.ld;

    if (!input || !output || !input->data || !output->data) {
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    DreamyRenderParams rp;
    std::memset(&rp, 0, sizeof(rp));

    rp.width       = output->width;
    rp.height      = output->height;
    rp.input_world = input;
    rp.output_world = output;
    rp.is_16bit    = PF_WORLD_IS_DEEP(output) ? TRUE : FALSE;

    if (in_data->time_scale > 0) {
        rp.time_sec = (float)in_data->current_time / (float)in_data->time_scale;
    }
    if (in_data->time_step > 0) {
        rp.frame_num = in_data->current_time / in_data->time_step;
    }
    if (in_data->downsample_x.den > 0 && in_data->downsample_x.num > 0) {
        rp.downsample_scale = (float)in_data->downsample_x.num / (float)in_data->downsample_x.den;
    } else {
        rp.downsample_scale = 1.0f;
    }

    /* 1. Extract & Analyze Music Reactivity */
    float audio_slider   = (float)params[PARAM_AUDIO_SLIDER]->u.fs_d.value;
    float audio_gain     = (float)params[PARAM_AUDIO_GAIN]->u.fs_d.value * 0.01f;
    float bass_weight    = (float)params[PARAM_AUDIO_BASS_WEIGHT]->u.fs_d.value * 0.01f;
    float mid_weight     = (float)params[PARAM_AUDIO_MID_WEIGHT]->u.fs_d.value * 0.01f;
    float high_weight    = (float)params[PARAM_AUDIO_HIGH_WEIGHT]->u.fs_d.value * 0.01f;
    float smooth_ms      = (float)params[PARAM_AUDIO_SMOOTHING]->u.fs_d.value;

    DreamyDSP::AnalyzeAudioReactivity(
        in_data,
        rp,
        audio_slider,
        audio_gain,
        bass_weight,
        mid_weight,
        high_weight,
        smooth_ms);

    /* 2. Extract Gradient Palette Settings */
    A_long grad_mode = params[PARAM_GRADIENT_MODE]->u.pd.value;
    DreamyDSP::SetupGradientStops(
        rp,
        grad_mode,
        params[PARAM_COLOR_SHADOW]->u.cd.value,
        params[PARAM_COLOR_MID_LOW]->u.cd.value,
        params[PARAM_COLOR_MID_HIGH]->u.cd.value,
        params[PARAM_COLOR_HIGHLIGHT]->u.cd.value);

    rp.gradient_mix = (float)params[PARAM_GRADIENT_MIX]->u.fs_d.value * 0.01f;
    float manual_phase = (float)params[PARAM_GRADIENT_CYCLE]->u.fs_d.value * 0.01f;
    float audio_phase  = (float)params[PARAM_GRADIENT_AUDIO_SHIFT]->u.fs_d.value * 0.01f;
    rp.gradient_phase  = manual_phase + rp.audio_mid * audio_phase;

    /* 3. Extract FM & Rutt-Etra Settings */
    rp.fm_enable           = (A_Boolean)params[PARAM_FM_ENABLE]->u.bd.value;
    rp.fm_carrier_freq     = (float)params[PARAM_FM_CARRIER_FREQ]->u.fs_d.value;
    rp.fm_mod_depth        = (float)params[PARAM_FM_MOD_DEPTH]->u.fs_d.value * 0.01f;
    rp.fm_rutt_etra_z      = (float)params[PARAM_FM_RUTT_ETRA_Z]->u.fs_d.value * 0.01f;
    rp.fm_scanline_density = (float)params[PARAM_FM_SCANLINE_DENSITY]->u.fs_d.value;
    rp.fm_line_thickness   = (float)params[PARAM_FM_LINE_THICKNESS]->u.fs_d.value * 0.01f;
    rp.fm_audio_react      = (float)params[PARAM_FM_AUDIO_REACT]->u.fs_d.value * 0.01f;

    /* 4. Extract Circuit Bending Settings */
    rp.bend_enable         = (A_Boolean)params[PARAM_BEND_ENABLE]->u.bd.value;
    rp.bend_phase_fold     = (float)params[PARAM_BEND_PHASE_FOLD]->u.fs_d.value * 0.01f;
    rp.bend_xor_crush      = (float)params[PARAM_BEND_XOR_CRUSH]->u.fs_d.value * 0.01f;
    rp.bend_smear_tear     = (float)params[PARAM_BEND_SMEAR_TEAR]->u.fs_d.value * 0.01f;
    rp.bend_feedback_shift = (float)params[PARAM_BEND_FEEDBACK_SHIFT]->u.fs_d.value * 0.01f;
    rp.bend_audio_react    = (float)params[PARAM_BEND_AUDIO_REACT]->u.fs_d.value * 0.01f;

    /* 5. Extract Dreamy VHS Settings */
    rp.vhs_enable          = (A_Boolean)params[PARAM_VHS_ENABLE]->u.bd.value;
    rp.vhs_dream_bloom     = (float)params[PARAM_VHS_DREAM_BLOOM]->u.fs_d.value * 0.01f;
    rp.vhs_chroma_bleed    = (float)params[PARAM_VHS_CHROMA_BLEED]->u.fs_d.value * 0.01f;
    rp.vhs_tape_wobble     = (float)params[PARAM_VHS_TAPE_WOBBLE]->u.fs_d.value * 0.01f;
    rp.vhs_tracking_noise  = (float)params[PARAM_VHS_TRACKING_NOISE]->u.fs_d.value * 0.01f;
    rp.vhs_grain           = (float)params[PARAM_VHS_GRAIN]->u.fs_d.value * 0.01f;

    /* 6. Extract CRT Scanlines & Phosphor Mask Settings */
    rp.crt_enable             = (A_Boolean)params[PARAM_CRT_ENABLE]->u.bd.value;
    rp.crt_scanline_pitch     = (float)params[PARAM_CRT_SCANLINE_PITCH]->u.fs_d.value;
    rp.crt_scanline_intensity = (float)params[PARAM_CRT_SCANLINE_INTENSITY]->u.fs_d.value * 0.01f;
    rp.crt_phosphor_mask      = params[PARAM_CRT_PHOSPHOR_MASK]->u.pd.value;
    rp.crt_phosphor_strength  = (float)params[PARAM_CRT_PHOSPHOR_STRENGTH]->u.fs_d.value * 0.01f;
    rp.crt_curvature          = (float)params[PARAM_CRT_CURVATURE]->u.fs_d.value * 0.01f;

    /* 7. Dispatch Multi-Threaded Scanline Render via AE's iterate_generic */
    if (in_data->utils && in_data->utils->iterate_generic) {
        err = PF_ITERATE_GENERIC(rp.height, &rp, RenderRowTask);
    } else {
        /* Fallback single-thread loop if host doesn't provide iterate_generic */
        for (A_long y = 0; y < rp.height; ++y) {
            err = RenderRowTask(&rp, 0, y, rp.height);
            if (err != PF_Err_NONE) break;
        }
    }

    return err;
}

/*------------------------------------------------------------------
 * PF_Cmd_UPDATE_PARAMS_UI: Enable/Disable custom 4-stop color pickers
 * when user switches between 'Custom 4-Stop Gradient' and a preset.
 *------------------------------------------------------------------*/
static PF_Err
UpdateParameterUI(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    PF_Err err = PF_Err_NONE;
    if (!params[PARAM_GRADIENT_MODE]) {
        return PF_Err_NONE;
    }

    A_Boolean is_custom = (params[PARAM_GRADIENT_MODE]->u.pd.value == GRAD_MODE_CUSTOM) ? TRUE : FALSE;

    /* Note: Even when a preset is selected, we leave the 4 color pickers visible
     * so the user can always customize them or switch back to Custom seamlessly. */
    (void)is_custom;
    return err;
}

/*------------------------------------------------------------------
 * Main Entry Point Required by Adobe After Effects
 *------------------------------------------------------------------*/
extern "C" DllExport PF_Err
EffectMain(
    PF_Cmd          cmd,
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output,
    void            *extra)
{
    PF_Err err = PF_Err_NONE;

    if (cmd != PF_Cmd_RENDER) {
        DreamyLog("EffectMain enter: cmd=%d (%s)", (int)cmd, CmdToString(cmd));
    }

    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:
                err = About(in_data, out_data, params, output);
                break;

            case PF_Cmd_GLOBAL_SETUP:
                err = GlobalSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_PARAMS_SETUP:
                err = ParamsSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_RENDER:
                err = Render(in_data, out_data, params, output);
                if (err != PF_Err_NONE) {
                    DreamyLog("Render returned error=%d", (int)err);
                }
                break;

            case PF_Cmd_UPDATE_PARAMS_UI:
                err = UpdateParameterUI(in_data, out_data, params, output);
                break;

            default:
                break;
        }
    } catch (...) {
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
        DreamyLog("EXCEPTION caught in EffectMain during cmd=%d (%s)!", (int)cmd, CmdToString(cmd));
    }

    if (cmd != PF_Cmd_RENDER || err != PF_Err_NONE) {
        DreamyLog("EffectMain exit: cmd=%d (%s) -> err=%d", (int)cmd, CmdToString(cmd), (int)err);
    }

    return err;
}
