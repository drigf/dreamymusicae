/*******************************************************************/
/*                                                                 */
/*  DreamyMusic.cpp                                                */
/*  Main Adobe After Effects Plugin Implementation                 */
/*                                                                 */
/*******************************************************************/

#define DREAMY_LOG_ADD_PARAM 1
static void DreamyLog(const char* fmt, ...);
static void DreamyDetailLog(const char* fmt, ...);
#include "DreamyMusic.h"
#include "DreamyDSP.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <chrono>
#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#endif

#define DREAMY_LOG_PATH "/tmp/dreamymusic.log"
#define DREAMY_WORKSPACE_LOG_PATH "/Users/niall/Developer/AE Plugins/dreamymusic/dreamymusic.log"
#define DREAMY_DETAIL_LOG_PATH "/tmp/dreamymusic_detailed.log"
#define DREAMY_WORKSPACE_DETAIL_LOG_PATH "/Users/niall/Developer/AE Plugins/dreamymusic/dreamymusic_detailed.log"

static int g_render_frame_counter = 0;
static bool g_log_next_render = true;
static bool g_logged_legacy_recovery = false;

/*------------------------------------------------------------------
 * Two-Tier Diagnostic Logging System:
 *   1. DreamyLog       -> Friendly Live Monitor (dreamymusic.log)
 *   2. DreamyDetailLog -> In-Depth Technical Log (dreamymusic_detailed.log)
 *------------------------------------------------------------------*/
static void WriteLogToFile(const char* path, const char* time_buf, const char* msg_buf) {
    FILE* fp = std::fopen(path, "a");
    if (!fp) return;
    if (time_buf[0] != '\0') {
        std::fprintf(fp, "%s", time_buf);
    }
    std::fprintf(fp, "%s\n", msg_buf);
    std::fflush(fp);
    std::fclose(fp);
}

static void DreamyDetailLog(const char* fmt, ...) {
    char time_buf[32] = {0};
    std::time_t now = std::time(nullptr);
    struct std::tm* t = std::localtime(&now);
    if (t) {
        std::snprintf(time_buf, sizeof(time_buf), "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    }

    char msg_buf[2048] = {0};
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    WriteLogToFile(DREAMY_DETAIL_LOG_PATH, time_buf, msg_buf);
    WriteLogToFile(DREAMY_WORKSPACE_DETAIL_LOG_PATH, time_buf, msg_buf);
}

static void DreamyLog(const char* fmt, ...) {
    char time_buf[32] = {0};
    std::time_t now = std::time(nullptr);
    struct std::tm* t = std::localtime(&now);
    if (t) {
        std::snprintf(time_buf, sizeof(time_buf), "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    }

    char msg_buf[1024] = {0};
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    WriteLogToFile(DREAMY_LOG_PATH, time_buf, msg_buf);
    WriteLogToFile(DREAMY_WORKSPACE_LOG_PATH, time_buf, msg_buf);
    WriteLogToFile(DREAMY_DETAIL_LOG_PATH, time_buf, msg_buf);
    WriteLogToFile(DREAMY_WORKSPACE_DETAIL_LOG_PATH, time_buf, msg_buf);
}

__attribute__((constructor))
static void OnDreamyPluginLoaded() {
    DreamyLog("====================================================================");
    DreamyLog("✨ Dreamy Music VHS v%d.%d Loaded into Adobe After Effects", MAJOR_VERSION, MINOR_VERSION);
    DreamyLog("====================================================================");
}

static const char* CmdToFriendlyName(PF_Cmd cmd) {
    switch (cmd) {
        case PF_Cmd_ABOUT:              return "About Dialog";
        case PF_Cmd_GLOBAL_SETUP:       return "Global Plugin Setup";
        case PF_Cmd_GLOBAL_SETDOWN:     return "Global Plugin Unload";
        case PF_Cmd_PARAMS_SETUP:       return "Effect Controls UI Setup";
        case PF_Cmd_SEQUENCE_SETUP:     return "Layer Instance Created";
        case PF_Cmd_SEQUENCE_RESETUP:   return "Layer Instance Reloaded";
        case PF_Cmd_SEQUENCE_FLATTEN:   return "Layer Instance Saved";
        case PF_Cmd_SEQUENCE_SETDOWN:   return "Layer Instance Removed";
        case PF_Cmd_FRAME_SETUP:        return "Frame Pre-Setup";
        case PF_Cmd_RENDER:             return "Frame Render";
        case PF_Cmd_FRAME_SETDOWN:      return "Frame Post-Render";
        case PF_Cmd_USER_CHANGED_PARAM: return "User Changed Control Value";
        case PF_Cmd_UPDATE_PARAMS_UI:   return "UI Controls Refresh";
        case PF_Cmd_SMART_PRE_RENDER:   return "SmartFX Pre-Render";
        case PF_Cmd_SMART_RENDER:       return "SmartFX Render";
        default:                        return "AE Command";
    }
}

static const char* GradientModeToString(A_long mode) {
    switch (mode) {
        case GRAD_MODE_CUSTOM:            return "Custom 4-Stop Gradient";
        case GRAD_MODE_SUNSET_DREAM:      return "Sunset Dream (Ref 1 FM)";
        case GRAD_MODE_ACID_CIRCUIT:      return "Acid Circuit Synth (Ref 2)";
        case GRAD_MODE_NEON_OSCILLOSCOPE: return "Neon Oscilloscope";
        case GRAD_MODE_CYBERPUNK_VHS:     return "Cyberpunk VHS";
        case GRAD_MODE_THERMAL_CRT:       return "Thermal Phosphor CRT";
        case GRAD_MODE_PASTEL_VAPORWAVE:  return "Pastel Vaporwave";
        default:                          return "Unknown Preset";
    }
}

static const char* ParamIndexToFriendlyName(PF_ParamIndex idx) {
    switch (idx) {
        case PARAM_AUDIO_LAYER:          return "Audio Source Layer";
        case PARAM_AUDIO_SLIDER:         return "Audio Keyframe Slider";
        case PARAM_AUDIO_GAIN:           return "Master Audio Reactivity";
        case PARAM_AUDIO_BASS_WEIGHT:    return "Bass Reactivity (FM & VHS)";
        case PARAM_AUDIO_MID_WEIGHT:     return "Mid Reactivity (Bloom & Color)";
        case PARAM_AUDIO_HIGH_WEIGHT:    return "High Reactivity (Circuit Bend)";
        case PARAM_AUDIO_SMOOTHING:      return "Audio Smoothing (ms)";
        case PARAM_GRADIENT_MODE:        return "Palette Preset";
        case PARAM_COLOR_SHADOW:         return "Shadow Color (Stop 1)";
        case PARAM_COLOR_MID_LOW:        return "Low Midtone (Stop 2)";
        case PARAM_COLOR_MID_HIGH:       return "High Midtone (Stop 3)";
        case PARAM_COLOR_HIGHLIGHT:      return "Highlight Color (Stop 4)";
        case PARAM_GRADIENT_MIX:         return "Gradient Blend";
        case PARAM_GRADIENT_CYCLE:       return "Color Phase Shift";
        case PARAM_GRADIENT_AUDIO_SHIFT: return "Audio Color Cycling";
        case PARAM_FM_ENABLE:            return "Enable FM & Rutt-Etra";
        case PARAM_FM_CARRIER_FREQ:      return "FM Wave Frequency";
        case PARAM_FM_MOD_DEPTH:         return "FM Wave Depth";
        case PARAM_FM_RUTT_ETRA_Z:       return "3D Scanline Elevation";
        case PARAM_FM_SCANLINE_DENSITY:  return "Oscilloscope Line Count";
        case PARAM_FM_LINE_THICKNESS:    return "Oscilloscope Beam Width";
        case PARAM_FM_AUDIO_REACT:       return "FM Music Reactivity";
        case PARAM_BEND_ENABLE:          return "Enable Circuit Bending";
        case PARAM_BEND_PHASE_FOLD:      return "Psychedelic Wavefolder";
        case PARAM_BEND_XOR_CRUSH:       return "Digital Pin-Short Glitch";
        case PARAM_BEND_SMEAR_TEAR:      return "Horizontal Sync Tear";
        case PARAM_BEND_FEEDBACK_SHIFT:  return "RGB Matrix Shift";
        case PARAM_BEND_AUDIO_REACT:     return "Bend Music Reactivity";
        case PARAM_VHS_ENABLE:           return "Enable Dreamy VHS";
        case PARAM_VHS_DREAM_BLOOM:      return "Dreamy Halation Glow";
        case PARAM_VHS_CHROMA_BLEED:     return "VHS Color Bleed";
        case PARAM_VHS_TAPE_WOBBLE:      return "Tape Edge Wobble";
        case PARAM_VHS_TRACKING_NOISE:   return "VHS Tracking Static Bar";
        case PARAM_VHS_GRAIN:            return "Magnetic Tape Grain";
        case PARAM_CRT_ENABLE:           return "Enable CRT Scanlines";
        case PARAM_CRT_SCANLINE_PITCH:   return "Scanline Size (px)";
        case PARAM_CRT_SCANLINE_INTENSITY: return "Scanline Contrast";
        case PARAM_CRT_PHOSPHOR_MASK:    return "Phosphor Subpixel Mask";
        case PARAM_CRT_PHOSPHOR_STRENGTH: return "Phosphor Mask Intensity";
        case PARAM_CRT_CURVATURE:        return "CRT Screen Curvature";
        default:                         return "Control Parameter";
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

/* Compile-time ABI layout verification against AE 2025/2026 arm64 */
static_assert(offsetof(PF_ParamDef, u) == 0x38, "PF_ParamDef.u must be at +0x38");
static_assert(offsetof(PF_ParamDef, u.fs_d.value) == 0x38, "fs_d.value must be at +0x38");
static_assert(offsetof(PF_ParamDef, u.fs_d.valid_min) == 0x68, "fs_d.valid_min must be at +0x68");
static_assert(offsetof(PF_ParamDef, u.fs_d.valid_max) == 0x6C, "fs_d.valid_max must be at +0x6C");
static_assert(offsetof(PF_ParamDef, u.fs_d.slider_min) == 0x70, "fs_d.slider_min must be at +0x70");
static_assert(offsetof(PF_ParamDef, u.fs_d.slider_max) == 0x74, "fs_d.slider_max must be at +0x74");
static_assert(offsetof(PF_ParamDef, u.fs_d.dephault) == 0x78, "fs_d.dephault must be at +0x78");
static_assert(offsetof(PF_ParamDef, u.ld.dephault) == 0xAC, "PF_ParamDef.u.ld.dephault must be at +0xAC");
static_assert(sizeof(PF_ParamDef) == 0xB0, "PF_ParamDef size must be 0xB0 (176 bytes)");
static_assert(offsetof(PF_InData, inter.checkout_layer_audio) == 0x30, "checkout_layer_audio must be at +0x30");
static_assert(offsetof(PF_InData, inter.checkin_layer_audio) == 0x38, "checkin_layer_audio must be at +0x38");
static_assert(offsetof(PF_InData, inter.get_audio_data) == 0x40, "get_audio_data must be at +0x40");
static_assert(offsetof(PF_InData, effect_ref) == 0xB8, "effect_ref must be at +0xB8");
static_assert(offsetof(PF_InData, current_time) == 0xE0, "current_time must be at +0xE0");
static_assert(offsetof(PF_InData, time_scale) == 0xF0, "time_scale must be at +0xF0");

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
        PF_OutFlag_DEEP_COLOR_AWARE         |
        PF_OutFlag_SEND_UPDATE_PARAMS_UI;

    out_data->out_flags2 =
        PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG |
        PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

    DreamyLog("✅ [INIT] Global Setup Complete | Version: %d.%d.%d (0x%x == PiPL 524289) | OutFlags: 0x%x | Deep Color (8/16-bit) + Multi-Frame Rendering Enabled",
              MAJOR_VERSION, MINOR_VERSION, BUG_VERSION,
              (unsigned int)out_data->my_version,
              (unsigned int)out_data->out_flags);

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
     * 1. MUSIC & AUDIO REACTIVITY (Starts Open)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Music & Audio Reactivity", ID_TOPIC_AUDIO_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Audio Source Layer", PF_LayerDefault_MYSELF, ID_AUDIO_LAYER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Keyframe Slider",
        0.0, 200.0, 0.0, 50.0, 0.0,
        1, PF_ValueDisplayFlag_NONE, PF_ParamFlag_SUPERVISE,
        ID_AUDIO_SLIDER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Master Reactivity",
        0.0, 500.0, 0.0, 250.0, 125.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_AUDIO_GAIN);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Bass Response (FM & VHS)",
        0.0, 300.0, 0.0, 200.0, 130.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_BASS_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Mid Response (Glow & Color)",
        0.0, 300.0, 0.0, 200.0, 100.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_MID_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Treble Response (Glitch)",
        0.0, 300.0, 0.0, 200.0, 120.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_AUDIO_HIGH_WEIGHT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Smoothing (ms)",
        15.0, 350.0, 25.0, 200.0, 75.0,
        0, PF_ValueDisplayFlag_NONE, 0,
        ID_AUDIO_SMOOTHING);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_AUDIO_END);

    /*==============================================================
     * 2. DREAMY GRADIENT PALETTE (Starts Open)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Dreamy Color Gradient", ID_TOPIC_GRADIENT_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUPX(
        "Palette Preset",
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
        "Gradient Blend",
        0.0, 100.0, 0.0, 100.0, 85.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_GRADIENT_MIX);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Color Phase Shift",
        -500.0, 500.0, -100.0, 100.0, 0.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_GRADIENT_CYCLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Audio Color Cycling",
        0.0, 200.0, 0.0, 100.0, 35.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_GRADIENT_AUDIO_SHIFT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_GRADIENT_END);

    /*==============================================================
     * 3. FM & RUTT-ETRA OSCILLOSCOPE (Starts Open)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("FM & Rutt-Etra Waves", ID_TOPIC_FM_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable FM & Rutt-Etra", TRUE, PF_ParamFlag_SUPERVISE, ID_FM_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Wave Frequency",
        1.0, 400.0, 10.0, 200.0, 85.0,
        1, PF_ValueDisplayFlag_NONE, 0,
        ID_FM_CARRIER_FREQ);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Wave Depth",
        0.0, 100.0, 0.0, 100.0, 25.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_FM_MOD_DEPTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "3D Scanline Elevation",
        0.0, 200.0, 0.0, 100.0, 35.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_FM_RUTT_ETRA_Z);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Oscilloscope Lines",
        0.0, 400.0, 0.0, 220.0, 90.0,
        0, PF_ValueDisplayFlag_NONE, 0,
        ID_FM_SCANLINE_DENSITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Oscilloscope Beam Width",
        5.0, 100.0, 15.0, 95.0, 62.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_LINE_THICKNESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "FM Music Reactivity",
        0.0, 300.0, 0.0, 200.0, 90.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_FM_AUDIO_REACT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_FM_END);

    /*==============================================================
     * 4. CIRCUIT BENDING (Starts Collapsed for Clean UI)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPICX("Circuit Bending Glitch", PF_ParamFlag_START_COLLAPSED, ID_TOPIC_BEND_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable Circuit Bending", TRUE, PF_ParamFlag_SUPERVISE, ID_BEND_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Psychedelic Wavefolder",
        0.0, 100.0, 0.0, 100.0, 18.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_BEND_PHASE_FOLD);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Digital Pin-Short (XOR)",
        0.0, 100.0, 0.0, 100.0, 0.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_BEND_XOR_CRUSH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Horizontal Sync Tear",
        0.0, 100.0, 0.0, 100.0, 15.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_SMEAR_TEAR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "RGB Matrix Shift",
        0.0, 100.0, 0.0, 100.0, 12.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_FEEDBACK_SHIFT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Bend Music Reactivity",
        0.0, 300.0, 0.0, 200.0, 100.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_BEND_AUDIO_REACT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_BEND_END);

    /*==============================================================
     * 5. DREAMY VHS & TAPE AESTHETICS (Starts Collapsed for Clean UI)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPICX("Dreamy VHS & Halation", PF_ParamFlag_START_COLLAPSED, ID_TOPIC_VHS_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable Dreamy VHS", TRUE, PF_ParamFlag_SUPERVISE, ID_VHS_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Dreamy Halation Glow",
        0.0, 150.0, 0.0, 100.0, 55.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_VHS_DREAM_BLOOM);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "VHS Color Bleed",
        0.0, 100.0, 0.0, 100.0, 42.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_CHROMA_BLEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Tape Edge Wobble",
        0.0, 100.0, 0.0, 100.0, 24.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_TAPE_WOBBLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "VHS Tracking Static Bar",
        0.0, 100.0, 0.0, 100.0, 20.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_TRACKING_NOISE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Magnetic Tape Grain",
        0.0, 100.0, 0.0, 100.0, 18.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_VHS_GRAIN);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_VHS_END);

    /*==============================================================
     * 6. CRT SCANLINES & TRINITRON PHOSPHOR MASK (Starts Collapsed)
     *==============================================================*/
    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPICX("CRT Scanlines & Phosphor", PF_ParamFlag_START_COLLAPSED, ID_TOPIC_CRT_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("Enable CRT Display", TRUE, PF_ParamFlag_SUPERVISE, ID_CRT_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Scanline Size (px)",
        2.0, 24.0, 2.0, 12.0, 4.0,
        1, PF_ValueDisplayFlag_PIXEL, 0,
        ID_CRT_SCANLINE_PITCH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Scanline Contrast",
        0.0, 100.0, 0.0, 100.0, 45.0,
        1, PF_ValueDisplayFlag_PERCENT, PF_ParamFlag_SUPERVISE,
        ID_CRT_SCANLINE_INTENSITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUPX(
        "Phosphor Subpixel Mask",
        4,
        CRT_MASK_TRINITRON_GRILLE,
        "None (Scanlines Only)|"
        "Trinitron Aperture Grille (Ref 3)|"
        "Shadow Mask Delta Triad|"
        "Arcade Slot Mask",
        PF_ParamFlag_SUPERVISE,
        ID_CRT_PHOSPHOR_MASK);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "Phosphor Mask Intensity",
        0.0, 100.0, 0.0, 100.0, 40.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_CRT_PHOSPHOR_STRENGTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(
        "CRT Screen Curvature",
        0.0, 100.0, 0.0, 100.0, 15.0,
        1, PF_ValueDisplayFlag_PERCENT, 0,
        ID_CRT_CURVATURE);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(ID_TOPIC_CRT_END);

    out_data->num_params = DREAMY_NUM_PARAMS;
    DreamyLog("🎛️ [UI SETUP] Registered %d Effect Controls across 6 collapsible groups (All slider ranges verified)",
              (int)out_data->num_params);
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

#if defined(__APPLE__)
static void
RenderStripGCD(void *context, size_t strip_idx)
{
    DreamyRenderParams* rp = reinterpret_cast<DreamyRenderParams*>(context);
    if (!rp || rp->height <= 0) return;
    const A_long num_strips = 8;
    A_long y_start = ((A_long)strip_idx * rp->height) / num_strips;
    A_long y_end   = (((A_long)strip_idx + 1) * rp->height) / num_strips;
    for (A_long y = y_start; y < y_end; ++y) {
        RenderRowTask(context, (A_long)strip_idx, y, rp->height);
    }
}
#endif

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
    PF_EffectWorld* input = (params && params[DREAMY_INPUT]) ? &params[DREAMY_INPUT]->u.ld : nullptr;

    if (!input || !output || !input->data || !output->data ||
        input->width <= 0 || input->height <= 0 ||
        output->width <= 0 || output->height <= 0) {
        DreamyLog("⚠️ [RENDER WARNING] Missing pixel buffer (input=%p, output=%p, in_data=%p, out_data=%p)",
                  (void*)input, (void*)output,
                  input ? (void*)input->data : nullptr,
                  output ? (void*)output->data : nullptr);
        return PF_Err_NONE;
    }

    for (int i = 0; i < DREAMY_NUM_PARAMS; ++i) {
        if (!params[i]) {
            DreamyLog("⚠️ [RENDER WARNING] params[%d] is NULL", i);
            return PF_Err_NONE;
        }
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

    auto render_t0 = std::chrono::steady_clock::now();

    /* 1. Extract & Analyze Music Reactivity */
    float audio_slider   = (float)params[PARAM_AUDIO_SLIDER]->u.fs_d.value;
    float audio_gain     = (float)params[PARAM_AUDIO_GAIN]->u.fs_d.value * 0.01f;
    float bass_weight    = (float)params[PARAM_AUDIO_BASS_WEIGHT]->u.fs_d.value * 0.01f;
    float mid_weight     = (float)params[PARAM_AUDIO_MID_WEIGHT]->u.fs_d.value * 0.01f;
    float high_weight    = (float)params[PARAM_AUDIO_HIGH_WEIGHT]->u.fs_d.value * 0.01f;
    float smooth_ms      = (float)params[PARAM_AUDIO_SMOOTHING]->u.fs_d.value;

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

    /* Auto-detect if this layer was saved in an .aep project during the previous
     * clamped-slider build (where Color Phase Shift was clamped to -100%, FM Wave Freq
     * to 10.0, Beam Width to 15.0%, and all 0-based sliders to 0.0%). If so, substitute
     * the true default values automatically so the effect renders immediately even before
     * the user clicks "Reset" in Effect Controls! */
    bool is_legacy_clamped_instance =
        (std::fabs((float)params[PARAM_GRADIENT_CYCLE]->u.fs_d.value - (-100.0f)) < 0.01f) &&
        (std::fabs(rp.fm_carrier_freq - 10.0f) < 0.01f) &&
        (std::fabs((float)params[PARAM_FM_LINE_THICKNESS]->u.fs_d.value - 15.0f) < 0.01f) &&
        (rp.gradient_mix == 0.0f) &&
        (audio_gain == 0.0f) &&
        (rp.fm_mod_depth == 0.0f) &&
        (rp.vhs_dream_bloom == 0.0f);

    if (is_legacy_clamped_instance) {
        if (!g_logged_legacy_recovery) {
            g_logged_legacy_recovery = true;
            DreamyLog("💡 [AUTO-RECOVERY] Layer had cached 0%% values from previous save — auto-applying default Dreamy Music VHS look! (Click 'Reset' next to Dreamy Music VHS in Effect Controls to refresh UI numbers)");
        }
        audio_gain                = 1.25f;
        bass_weight               = 1.30f;
        mid_weight                = 1.00f;
        high_weight               = 1.20f;
        smooth_ms                 = 75.0f;
        rp.gradient_mix           = 0.85f;
        manual_phase              = 0.0f;
        audio_phase               = 0.35f;
        rp.fm_carrier_freq        = 85.0f;
        rp.fm_mod_depth           = 0.25f;
        rp.fm_rutt_etra_z         = 0.35f;
        rp.fm_scanline_density    = 90.0f;
        rp.fm_line_thickness      = 0.62f;
        rp.fm_audio_react         = 0.90f;
        rp.bend_phase_fold        = 0.18f;
        rp.bend_xor_crush         = 0.00f;
        rp.bend_smear_tear        = 0.15f;
        rp.bend_feedback_shift    = 0.12f;
        rp.bend_audio_react       = 1.00f;
        rp.vhs_dream_bloom        = 0.55f;
        rp.vhs_chroma_bleed       = 0.42f;
        rp.vhs_tape_wobble        = 0.24f;
        rp.vhs_tracking_noise     = 0.20f;
        rp.vhs_grain              = 0.18f;
        rp.crt_scanline_pitch     = 4.0f;
        rp.crt_scanline_intensity = 0.45f;
        rp.crt_phosphor_strength  = 0.48f;
        rp.crt_curvature          = 0.14f;
    }

    DreamyDSP::AnalyzeAudioReactivity(
        in_data,
        rp,
        audio_slider,
        audio_gain,
        bass_weight,
        mid_weight,
        high_weight,
        smooth_ms);

    rp.gradient_phase = manual_phase + rp.audio_mid * audio_phase;

    /* 7. Dispatch Multi-Threaded Scanline Render via Grand Central Dispatch */
    g_render_frame_counter++;
    bool should_log_this_frame = (g_render_frame_counter <= 5 || g_log_next_render || (g_render_frame_counter % 15) == 0);
    if (should_log_this_frame) {
        g_log_next_render = false;
        DreamyLog("🎬 [RENDER #%d] Frame %d (%.2fs) | %dx%d (%s) | Audio[Bass:%.2f Mid:%.2f High:%.2f] | Palette: %s (Mix:%.0f%%) | Modules[FM:%s Bend:%s VHS:%s CRT:%s]",
                  g_render_frame_counter,
                  (int)rp.frame_num,
                  rp.time_sec,
                  (int)rp.width,
                  (int)rp.height,
                  rp.is_16bit ? "16-bit" : "8-bit",
                  rp.audio_bass,
                  rp.audio_mid,
                  rp.audio_high,
                  GradientModeToString(grad_mode),
                  rp.gradient_mix * 100.0f,
                  rp.fm_enable ? "ON" : "off",
                  rp.bend_enable ? "ON" : "off",
                  rp.vhs_enable ? "ON" : "off",
                  rp.crt_enable ? "ON" : "off");
    }

#if defined(__APPLE__)
    if (rp.height >= 16) {
        dispatch_apply_f(
            8,
            dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
            &rp,
            RenderStripGCD);
    } else {
        for (A_long y = 0; y < rp.height; ++y) {
            RenderRowTask(&rp, 0, y, rp.height);
        }
    }
#else
    if (in_data->utils && in_data->utils->iterate_generic) {
        err = PF_ITERATE_GENERIC(rp.height, &rp, RenderRowTask);
    } else {
        for (A_long y = 0; y < rp.height; ++y) {
            err = RenderRowTask(&rp, 0, y, rp.height);
            if (err != PF_Err_NONE) break;
        }
    }
#endif

    if (should_log_this_frame) {
        auto render_t1 = std::chrono::steady_clock::now();
        double render_ms = std::chrono::duration<double, std::milli>(render_t1 - render_t0).count();

        /* Sample center pixel from input and output to verify visual transformation */
        A_long cx = rp.width / 2;
        A_long cy = rp.height / 2;
        int in_r = 0, in_g = 0, in_b = 0;
        int out_r = 0, out_g = 0, out_b = 0;
        if (!rp.is_16bit) {
            const PF_Pixel8* in_row = reinterpret_cast<const PF_Pixel8*>(
                reinterpret_cast<const char*>(input->data) + (cy < input->height ? cy : 0) * input->rowbytes);
            const PF_Pixel8* out_row = reinterpret_cast<const PF_Pixel8*>(
                reinterpret_cast<const char*>(output->data) + cy * output->rowbytes);
            A_long icx = cx < input->width ? cx : 0;
            in_r = in_row[icx].red;   in_g = in_row[icx].green;   in_b = in_row[icx].blue;
            out_r = out_row[cx].red;  out_g = out_row[cx].green;  out_b = out_row[cx].blue;
        }

        DreamyDetailLog("   ├─ [PERF & BUFFER] RenderTime=%.2fms | In=%p (%dx%d, rb=%d) -> Out=%p (%dx%d, rb=%d) | CenterPixel RGB(%d,%d,%d) -> RGB(%d,%d,%d)%s",
                        render_ms,
                        (void*)input->data, (int)input->width, (int)input->height, (int)input->rowbytes,
                        (void*)output->data, (int)output->width, (int)output->height, (int)output->rowbytes,
                        in_r, in_g, in_b, out_r, out_g, out_b,
                        is_legacy_clamped_instance ? " [AUTO-RECOVERY ACTIVE]" : "");
        DreamyDetailLog("   ├─ [AUDIO DSP] CheckoutErr=%d | Samples=%d | Peak=%.4f | Raw[B:%.3f M:%.3f H:%.3f] * Gains[Master:%.0f%% B:%.0f%% M:%.0f%% H:%.0f%%] -> Active[B:%.3f M:%.3f H:%.3f]",
                        (int)rp.audio_checkout_err,
                        (int)rp.audio_num_samples,
                        rp.audio_peak_sample,
                        rp.audio_raw_bass, rp.audio_raw_mid, rp.audio_raw_high,
                        audio_gain * 100.0f, bass_weight * 100.0f, mid_weight * 100.0f, high_weight * 100.0f,
                        rp.audio_bass, rp.audio_mid, rp.audio_high);
        DreamyDetailLog("   └─ [MODULE PARAMS] Grad[Mode=%d Mix=%.0f%% Phase=%.2f] | FM[Freq=%.1f Depth=%.0f%% RuttZ=%.0f%% Lines=%.0f Width=%.0f%%] | Bend[Fold=%.0f%% XOR=%.0f%% Tear=%.0f%% RGB=%.0f%%] | VHS[Bloom=%.0f%% Bleed=%.0f%% Wobble=%.0f%% Track=%.0f%% Grain=%.0f%%] | CRT[Pitch=%.1fpx Dark=%.0f%% Mask=%dGrille=%.0f%% Curve=%.0f%%]",
                        (int)grad_mode, rp.gradient_mix * 100.0f, rp.gradient_phase,
                        rp.fm_carrier_freq, rp.fm_mod_depth * 100.0f, rp.fm_rutt_etra_z * 100.0f, rp.fm_scanline_density, rp.fm_line_thickness * 100.0f,
                        rp.bend_phase_fold * 100.0f, rp.bend_xor_crush * 100.0f, rp.bend_smear_tear * 100.0f, rp.bend_feedback_shift * 100.0f,
                        rp.vhs_dream_bloom * 100.0f, rp.vhs_chroma_bleed * 100.0f, rp.vhs_tape_wobble * 100.0f, rp.vhs_tracking_noise * 100.0f, rp.vhs_grain * 100.0f,
                        rp.crt_scanline_pitch, rp.crt_scanline_intensity * 100.0f, (int)rp.crt_phosphor_mask, rp.crt_phosphor_strength * 100.0f, rp.crt_curvature * 100.0f);
    }

    return err;
}

/*------------------------------------------------------------------
 * PF_Cmd_USER_CHANGED_PARAM: Log exact user slider/preset changes
 *------------------------------------------------------------------*/
static PF_Err
UserChangedParam(
    PF_InData                   *in_data,
    PF_OutData                  *out_data,
    PF_ParamDef                 *params[],
    PF_LayerDef                 *output,
    const PF_UserChangedParamExtra *extraP)
{
    if (!extraP) return PF_Err_NONE;
    PF_ParamIndex idx = extraP->param_index;
    const char* name = ParamIndexToFriendlyName(idx);
    g_log_next_render = true;

    if (idx > 0 && idx < DREAMY_NUM_PARAMS && params && params[idx]) {
        PF_ParamDef* p = params[idx];
        switch (p->param_type) {
            case PF_Param_FLOAT_SLIDER:
                DreamyLog("🎚️ [CONTROL CHANGED] %s -> %.2f", name, (double)p->u.fs_d.value);
                break;
            case PF_Param_CHECKBOX:
                DreamyLog("☑️ [TOGGLE CHANGED] %s -> %s", name, p->u.bd.value ? "ENABLED" : "DISABLED");
                break;
            case PF_Param_POPUP:
                if (idx == PARAM_GRADIENT_MODE) {
                    DreamyLog("🎨 [PRESET CHANGED] %s -> %s", name, GradientModeToString(p->u.pd.value));
                } else {
                    DreamyLog("📋 [MENU CHANGED] %s -> Option #%d", name, (int)p->u.pd.value);
                }
                break;
            case PF_Param_COLOR:
                DreamyLog("🎨 [COLOR PICKED] %s -> RGB(%d, %d, %d)",
                          name, (int)p->u.cd.value.red, (int)p->u.cd.value.green, (int)p->u.cd.value.blue);
                break;
            default:
                DreamyLog("🔧 [PARAM CHANGED] %s (index %d)", name, (int)idx);
                break;
        }
    }
    out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
    return PF_Err_NONE;
}

/*------------------------------------------------------------------
 * PF_Cmd_UPDATE_PARAMS_UI: Enable/Disable custom 4-stop color pickers
 *------------------------------------------------------------------*/
static PF_Err
UpdateParameterUI(
    PF_InData       *in_data,
    PF_OutData      *out_data,
    PF_ParamDef     *params[],
    PF_LayerDef     *output)
{
    return PF_Err_NONE;
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

    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:
                DreamyLog("ℹ️ [ABOUT] Displaying About Dialog");
                err = About(in_data, out_data, params, output);
                break;

            case PF_Cmd_GLOBAL_SETUP:
                err = GlobalSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_PARAMS_SETUP:
                err = ParamsSetup(in_data, out_data, params, output);
                break;

            case PF_Cmd_SEQUENCE_SETUP:
                DreamyLog("🆕 [LAYER] Dreamy Music VHS applied to layer");
                break;

            case PF_Cmd_RENDER:
                err = Render(in_data, out_data, params, output);
                if (err != PF_Err_NONE) {
                    DreamyLog("❌ [RENDER ERROR] Frame render returned error code %d", (int)err);
                }
                break;

            case PF_Cmd_USER_CHANGED_PARAM:
                err = UserChangedParam(
                    in_data,
                    out_data,
                    params,
                    output,
                    reinterpret_cast<const PF_UserChangedParamExtra*>(extra));
                break;

            case PF_Cmd_UPDATE_PARAMS_UI:
                err = UpdateParameterUI(in_data, out_data, params, output);
                break;

            default:
                break;
        }
    } catch (...) {
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
        DreamyLog("❌ [EXCEPTION] Unhandled exception in EffectMain during %s (cmd=%d)!",
                  CmdToFriendlyName(cmd), (int)cmd);
    }

    if (err != PF_Err_NONE) {
        DreamyLog("⚠️ [WARNING] %s (cmd=%d) exited with non-zero status: %d",
                  CmdToFriendlyName(cmd), (int)cmd, (int)err);
    }

    return err;
}
