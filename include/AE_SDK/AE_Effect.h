/*******************************************************************/
/*                                                                 */
/*  Minimal Self-Contained Adobe After Effects SDK Header Subset   */
/*  Compatible with Adobe After Effects 2022 - 2026 (macOS / Win)  */
/*  Drop-in compatible with official AfterEffectsSDK/Examples/Headers */
/*                                                                 */
/*******************************************************************/

#ifndef _H_AE_EFFECT
#define _H_AE_EFFECT

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__APPLE__)
    #define AE_OS_MAC 1
    #define DllExport __attribute__((visibility("default")))
#elif defined(_WIN32)
    #define AE_OS_WIN 1
    #define DllExport __declspec(dllexport)
#else
    #define DllExport
#endif

/* Basic Types */
typedef int32_t         A_long;
typedef uint32_t        A_u_long;
typedef int16_t         A_short;
typedef uint16_t        A_u_short;
typedef char            A_char;
typedef uint8_t         A_u_char;
typedef uint8_t         A_Boolean;
typedef double          A_FpLong;
typedef float           A_FpShort;
typedef int32_t         A_Fixed;
typedef int64_t         A_int64;
typedef uint64_t        A_u_int64;
typedef void*           A_Handle;

typedef A_long          PF_Err;
typedef A_long          A_Err;

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

/* Fixed point macros (16.16) */
#define PF_Fixed_ONE            (0x00010000L)
#define PF_Fixed_HALF           (0x00008000L)
#define INT2FIX(X)              ((A_Fixed)((X) << 16))
#define FIX2INT(X)              ((A_long)((X) >> 16))
#define FLT2FIX(F)              ((A_Fixed)((F) * 65536.0 + (((F) < 0) ? -0.5 : 0.5)))
#define FIX_2_FLOAT(X)          ((double)(X) / 65536.0)

/* Error Codes */
enum {
    PF_Err_NONE = 0,
    PF_Err_OUT_OF_MEMORY = 4,
    PF_Err_INTERNAL_STRUCT_DAMAGED = 512,
    PF_Err_INVALID_INDEX = 513,
    PF_Err_UNRECOGNIZED_PARAM_TYPE = 514,
    PF_Err_CANNOT_PARSE_KEYFRAME_TEXT = 515,
    PF_Err_BAD_CALLBACK_PARAM = 516
};

#define A_Err_NONE 0

/* Commands */
enum {
    PF_Cmd_ABOUT = 0,
    PF_Cmd_GLOBAL_SETUP,
    PF_Cmd_UNUSED_0,
    PF_Cmd_GLOBAL_SETDOWN,
    PF_Cmd_PARAMS_SETUP,
    PF_Cmd_SEQUENCE_SETUP,
    PF_Cmd_SEQUENCE_RESETUP,
    PF_Cmd_SEQUENCE_FLATTEN,
    PF_Cmd_SEQUENCE_SETDOWN,
    PF_Cmd_DO_DIALOG,
    PF_Cmd_FRAME_SETUP,
    PF_Cmd_RENDER,
    PF_Cmd_FRAME_SETDOWN,
    PF_Cmd_USER_CHANGED_PARAM,
    PF_Cmd_UPDATE_PARAMS_UI,
    PF_Cmd_EVENT,
    PF_Cmd_GET_EXTERNAL_DEPENDENCIES,
    PF_Cmd_COMPLETELY_GENERAL,
    PF_Cmd_QUERY_DYNAMIC_FLAGS,
    PF_Cmd_AUDIO_RENDER,
    PF_Cmd_AUDIO_SETUP,
    PF_Cmd_AUDIO_SETDOWN,
    PF_Cmd_SMART_PRE_RENDER,
    PF_Cmd_SMART_RENDER
};
typedef A_long PF_Cmd;

/* OutFlags */
enum {
    PF_OutFlag_NONE                         = 0,
    PF_OutFlag_KEEP_RESOURCE_OPEN           = 1L << 0,
    PF_OutFlag_WIDE_TIME_INPUT              = 1L << 1,
    PF_OutFlag_NON_PARAM_VARY               = 1L << 2,
    PF_OutFlag_RESERVED6                    = 1L << 3,
    PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING = 1L << 4,
    PF_OutFlag_I_DO_DIALOG                  = 1L << 5,
    PF_OutFlag_USE_OUTPUT_EXTENT            = 1L << 6,
    PF_OutFlag_SEND_DO_DIALOG               = 1L << 7,
    PF_OutFlag_DISPLAY_ERROR_MESSAGE        = 1L << 8,
    PF_OutFlag_I_EXPAND_BUFFER              = 1L << 9,
    PF_OutFlag_PIX_INDEPENDENT              = 1L << 10,
    PF_OutFlag_I_WRITE_INPUT_BUFFER         = 1L << 11,
    PF_OutFlag_I_SHRINK_BUFFER              = 1L << 12,
    PF_OutFlag_WORKS_IN_PLACE               = 1L << 13,
    PF_OutFlag_CUSTOM_UI                    = 1L << 14,
    PF_OutFlag_REFRESH_UI                   = 1L << 15,
    PF_OutFlag_NOP_RENDER                   = 1L << 16,
    PF_OutFlag_I_USE_SHUTTER_ANGLE          = 1L << 17,
    PF_OutFlag_I_USE_AUDIO                  = 1L << 18,
    PF_OutFlag_I_AM_OBSOLETE                = 1L << 19,
    PF_OutFlag_FORCE_RERENDER               = 1L << 20,
    PF_OutFlag_PIPL_OVERRIDES_OUTDATA_OUTFLAGS = 1L << 21,
    PF_OutFlag_I_HAVE_EXTERNAL_DEPENDENCIES = 1L << 22,
    PF_OutFlag_DEEP_COLOR_AWARE             = 1L << 25,
    PF_OutFlag_SEND_UPDATE_PARAMS_UI        = 1L << 26
};

/* OutFlags2 */
enum {
    PF_OutFlag2_NONE                        = 0,
    PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS = 1L << 0,
    PF_OutFlag2_I_USE_3D_CAMERA             = 1L << 1,
    PF_OutFlag2_I_USE_3D_LIGHTS             = 1L << 2,
    PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG = 1L << 3,
    PF_OutFlag2_I_AM_THREADSAFE             = 1L << 4,
    PF_OutFlag2_CAN_COMBINE_WITH_DESTINATION = 1L << 5,
    PF_OutFlag2_DOESNT_NEED_EMPTY_PIXELS    = 1L << 6,
    PF_OutFlag2_REVEALS_ZERO_ALPHA          = 1L << 7,
    PF_OutFlag2_PRESERVES_FULLY_OPAQUE_PIXELS = 1L << 8,
    PF_OutFlag2_SUPPORTS_SMART_RENDER       = 1L << 10,
    PF_OutFlag2_FLOAT_COLOR_AWARE           = 1L << 12,
    PF_OutFlag2_I_USE_COLORSPACE_ENUMERATION = 1L << 13,
    PF_OutFlag2_I_AM_DEPRECATED             = 1L << 14,
    PF_OutFlag2_PPRO_DO_NOT_CLONE_SEQUENCE_DATA_FOR_RENDER = 1L << 15,
    PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT   = 1L << 17,
    PF_OutFlag2_I_USE_TIMECODE              = 1L << 18,
    PF_OutFlag2_DEPENDS_ON_UNREFERENCED_MASKS = 1L << 19,
    PF_OutFlag2_OUTPUT_IS_WATERMARKED       = 1L << 20,
    PF_OutFlag2_SUPPORTS_GPU_RENDER_F32     = 1L << 24,
    PF_OutFlag2_SUPPORTS_THREADED_RENDERING = 1L << 27
};

/* Versioning */
#define PF_PLUG_IN_VERSION          13
#define PF_PLUG_IN_SUBVERS          28

#define PF_VERSION(vers, subvers, bugvers, stage, build) \
    ((((A_u_long)(vers) & 0x7UL) << 19) | \
     (((A_u_long)(subvers) & 0xfUL) << 15) | \
     (((A_u_long)(bugvers) & 0xfUL) << 11) | \
     (((A_u_long)(stage) & 0x3UL) << 9) | \
     ((A_u_long)(build) & 0x1ffUL))

enum {
    PF_Stage_DEVELOP = 0,
    PF_Stage_ALPHA,
    PF_Stage_BETA,
    PF_Stage_RELEASE
};

/* Pixel Structures */
typedef struct {
    A_u_char    alpha;
    A_u_char    red;
    A_u_char    green;
    A_u_char    blue;
} PF_Pixel, PF_Pixel8;

typedef struct {
    A_u_short   alpha;
    A_u_short   red;
    A_u_short   green;
    A_u_short   blue;
} PF_Pixel16;

typedef struct {
    A_FpShort   alpha;
    A_FpShort   red;
    A_FpShort   green;
    A_FpShort   blue;
} PF_PixelFloat, PF_Pixel32;

#define PF_MAX_CHAN8            255
#define PF_HALF_CHAN8           128
#define PF_MAX_CHAN16           32768
#define PF_HALF_CHAN16          16384

/* Geometry & Rects */
typedef struct {
    A_long      left, top, right, bottom;
} PF_LRect, A_LRect;

typedef union {
    PF_LRect    limits;
    struct {
        A_Fixed x, y;
    } point;
} PF_UnionableRect;

typedef struct {
    A_Fixed     x, y;
} PF_FixedPoint;

typedef struct {
    A_FpLong    x, y;
} PF_FloatPoint;

typedef struct {
    A_long      num;    /* numerator */
    A_u_long    den;    /* denominator */
} PF_RationalScale;

typedef struct {
    A_long      value;
    A_u_long    scale;
} A_Time;

/* Effect World / Layer */
typedef struct PF_PixelOpaque* PF_PixelPtr;

enum {
    PF_WorldFlag_DEEP           = 1L << 0,
    PF_WorldFlag_WRITEABLE      = 1L << 1,
    PF_WorldFlag_RESERVED0      = 1L << 2,
    PF_WorldFlag_RESERVED1      = 1L << 3,
    PF_WorldFlag_RESERVED2      = 1L << 4,
    PF_WorldFlag_RESERVED3      = 1L << 5,
    PF_WorldFlag_RESERVED4      = 1L << 6,
    PF_WorldFlag_RESERVED5      = 1L << 7,
    PF_WorldFlag_RESERVED6      = 1L << 8,
    PF_WorldFlag_RESERVED       = 1L << 31
};

typedef struct PF_EffectWorld {
    void*       world_handle;
    PF_PixelPtr data;
    A_long      rowbytes;
    A_long      width;
    A_long      height;
    PF_UnionableRect extent_hint;
    A_long      pix_aspect_ratio_num;
    A_u_long    pix_aspect_ratio_den;
    A_long      world_flags;
    void*       platform_ref;
    A_long      reserved[8];
} PF_EffectWorld, PF_LayerDef;

#define PF_WORLD_IS_DEEP(W)     (((W)->world_flags & PF_WorldFlag_DEEP) != 0)

/* Audio Types */
typedef float   PF_SoundSampleFloat;
enum {
    PF_SSS_1 = 1,
    PF_SSS_2 = 2,
    PF_SSS_4 = 4
};
enum {
    PF_Channels_MONO = 1,
    PF_Channels_STEREO = 2
};
enum {
    PF_SIGNED_PCM = 1,
    PF_SIGNED_FLOAT = 2
};

typedef struct {
    A_FpLong    rateF;
    A_long      num_channels;
    A_long      format;
    A_long      sample_size;
} PF_SoundFormatInfo;

typedef struct PF_SoundWorld {
    PF_SoundFormatInfo  fi;
    A_long              num_samples;
    void*               dataP;
} PF_SoundWorld;

/* Parameter Types & Flags */
enum {
    PF_Param_LAYER = 0,
    PF_Param_SLIDER,
    PF_Param_FIX_SLIDER,
    PF_Param_ANGLE,
    PF_Param_CHECKBOX,
    PF_Param_COLOR,
    PF_Param_POINT,
    PF_Param_POPUP,
    PF_Param_CUSTOM,
    PF_Param_NO_DATA,
    PF_Param_FLOAT_SLIDER,
    PF_Param_ARBITRARY_DATA,
    PF_Param_PATH,
    PF_Param_GROUP_START,
    PF_Param_GROUP_END,
    PF_Param_BUTTON,
    PF_Param_RESERVED2,
    PF_Param_POINT_3D
};
typedef A_long PF_ParamType;

enum {
    PF_ParamFlag_RESERVED1              = 1 << 0,
    PF_ParamFlag_CANNOT_TIME_VARY       = 1 << 1,
    PF_ParamFlag_CANNOT_INTERP          = 1 << 2,
    PF_ParamFlag_RESERVED2              = 1 << 3,
    PF_ParamFlag_RESERVED3              = 1 << 4,
    PF_ParamFlag_COLLAPSE_TWIRLY        = 1 << 5,
    PF_ParamFlag_SUPERVISE              = 1 << 6,
    PF_ParamFlag_START_COLLAPSED        = PF_ParamFlag_COLLAPSE_TWIRLY,
    PF_ParamFlag_USE_VALUE_FOR_OLD_PROJECTS = 1 << 7,
    PF_ParamFlag_LAYER_PARAM_IS_TRACKMATTE  = 1 << 7,
    PF_ParamFlag_EXCLUDE_FROM_HAVE_INPUTS_CHANGED = 1 << 8,
    PF_ParamFlag_SKIP_REVEAL_WHEN_UNHIDDEN  = 1 << 9
};
typedef A_long PF_ParamFlags;

enum {
    PF_PUI_NONE                 = 0,
    PF_PUI_TOPIC                = 1L << 0,
    PF_PUI_CONTROL              = 1L << 1,
    PF_PUI_STD_CONTROL_ONLY     = 1L << 2,
    PF_PUI_NO_ECW_UI            = 1L << 3,
    PF_PUI_ECW_SEPARATOR        = 1L << 4,
    PF_PUI_DISABLED             = 1L << 5,
    PF_PUI_INVISIBLE            = 1L << 6
};
typedef A_long PF_ParamUIFlags;

enum {
    PF_ValueDisplayFlag_NONE        = 0,
    PF_ValueDisplayFlag_PERCENT     = 1 << 0,
    PF_ValueDisplayFlag_PIXEL       = 1 << 1,
    PF_ValueDisplayFlag_RESERVED1   = 1 << 2
};

enum {
    PF_ChangeFlag_NONE              = 0,
    PF_ChangeFlag_CHANGED_VALUE     = 1L << 0,
    PF_ChangeFlag_RESERVED          = 1L << 1,
    PF_ChangeFlag_SET_TO_DEFAULT    = 1L << 2
};
typedef A_long PF_ChangeFlags;

typedef A_long PF_ParamIndex;
typedef A_long PF_ParamValue;

typedef struct {
    PF_ParamValue   value;
    A_char          value_str[32];
    A_char          value_desc[32];
    PF_ParamValue   valid_min, slider_min;
    PF_ParamValue   valid_max, slider_max;
    PF_ParamValue   dephault;
} PF_SliderDef;

typedef struct {
    A_Fixed         value;
    A_char          value_str[32];
    A_char          value_desc[32];
    A_Fixed         valid_min, slider_min;
    A_Fixed         valid_max, slider_max;
    A_Fixed         dephault;
    A_short         precision;
    A_short         display_flags;
} PF_FixedSliderDef;

#define AE_FLOAT_SLIDER_PHASE_NONE 0
typedef struct {
    A_FpLong        value;
    A_FpLong        phase;
    A_char          value_desc[32];
    A_FpLong        valid_min, slider_min;
    A_FpLong        valid_max, slider_max;
    A_FpLong        dephault;
    A_short         precision;
    A_short         display_flags;
    A_u_long        curve_tolerance;
    A_u_long        flags;
} PF_FloatSliderDef;

typedef struct {
    A_Fixed         value;
    A_Fixed         dephault;
    A_Fixed         valid_min, valid_max;
} PF_AngleDef;

typedef struct {
    PF_ParamValue   value;
    A_Boolean       dephault;
    A_char          reserved;
    A_short         reserved1;
    union {
        const A_char    *nameptr;
    } u;
} PF_CheckBoxDef;

typedef struct {
    PF_Pixel        value;
    PF_Pixel        dephault;
} PF_ColorDef;

typedef struct {
    A_Fixed         x_value, y_value;
    A_char          reserved[3];
    A_Boolean       restrict_bounds;
    A_Fixed         x_dephault, y_dephault;
} PF_PointDef;

typedef struct {
    PF_ParamValue   value;
    A_short         num_choices;
    A_short         dephault;
    union {
        const A_char    *namesptr;
    } u;
} PF_PopupDef;

typedef struct {
    union {
        const A_char    *nameptr;
    } u;
} PF_ButtonDef;

enum {
    PF_LayerDefault_MYSELF = -1,
    PF_LayerDefault_NONE = 0
};

typedef struct {
    PF_LayerDef     ld;
    A_long          dephault;
} PF_LayerParamDef;

typedef union {
    PF_LayerDef         ld;
    PF_SliderDef        sd;
    PF_FixedSliderDef   fd;
    PF_AngleDef         ad;
    PF_CheckBoxDef      bd;
    PF_ColorDef         cd;
    PF_PointDef         td;
    PF_PopupDef         pd;
    PF_FloatSliderDef   fs_d;
    PF_ButtonDef        button_d;
    PF_LayerParamDef    layer_d;
} PF_ParamDefUnion;

typedef struct {
    union {
        A_long          id;
        PF_ChangeFlags  change_flags;
    } uu;
    PF_ParamUIFlags ui_flags;
    A_short         ui_width;
    A_short         ui_height;
    PF_ParamType    param_type;
    A_char          name[32];
    PF_ParamFlags   flags;
    A_long          unused;
    PF_ParamDefUnion u;
} PF_ParamDef, *PF_ParamDefPtr, **PF_ParamDefH;

typedef PF_ParamDef** PF_ParamList;

/* Forward declarations for callbacks */
struct PF_InData;
struct SPBasicSuite;

typedef struct {
    PF_Err (*checkout_param)(
        void*           effect_ref,
        PF_ParamIndex   index,
        A_long          what_time,
        A_long          time_step,
        A_u_long        time_scale,
        PF_ParamDef     *param);

    PF_Err (*checkin_param)(
        void*           effect_ref,
        PF_ParamDef     *param);

    PF_Err (*add_param)(
        void*           effect_ref,
        PF_ParamIndex   index,
        PF_ParamDefPtr  def);

    PF_Err (*abort)(
        void*           effect_ref);

    PF_Err (*progress)(
        void*           effect_ref,
        A_long          current,
        A_long          total);

    PF_Err (*register_ui)(
        void*           effect_ref,
        void*           custom_ui_info);

    PF_Err (*checkout_layer_audio)(
        void*               effect_ref,
        PF_ParamIndex       index,
        A_long              start_time,
        A_long              duration,
        A_u_long            time_scale,
        A_FpLong            rate,
        A_long              bytes_per_sample,
        A_long              num_channels,
        A_long              fmt_signed,
        void**              audio);

    PF_Err (*checkin_layer_audio)(
        void*               effect_ref,
        void*               audio);

    PF_Err (*get_audio_data)(
        void*               effect_ref,
        void*               audio,
        PF_SoundSampleFloat **data0,
        A_long              *num_samples0,
        A_FpLong            *rate0,
        A_long              *bytes_per_sample0,
        A_long              *num_channels0,
        A_long              *fmt_signed0);

    void* reserved[24];
} PF_InteractCallbacks;

typedef struct {
    PF_Err (*begin_sampling)(void* effect_ref, A_long qual, A_long mf, void* params);
    PF_Err (*subpixel_sample)(void* effect_ref, A_Fixed x, A_Fixed y, const void* params, PF_Pixel* dst_pixel);
    PF_Err (*area_sample)(void* effect_ref, A_Fixed x, A_Fixed y, const void* params, PF_Pixel* dst_pixel);
    void*  reserved_0;
    PF_Err (*end_sampling)(void* effect_ref, A_long qual, A_long mf, void* params);
    PF_Err (*composite_rect)(void* effect_ref, PF_LRect* src_rect, A_long src_opacity, PF_EffectWorld* src_w, A_long dst_x, A_long dst_y, A_long field, A_long xfer, PF_EffectWorld* dst);
    PF_Err (*blend)(void* effect_ref, const PF_EffectWorld* src1, const PF_EffectWorld* src2, A_Fixed ratio, PF_EffectWorld* dst);
    PF_Err (*convolve)(void* effect_ref, PF_EffectWorld* src, const PF_LRect* area, A_long flags, A_long kernel_size, void* a_kernel, void* r_kernel, void* g_kernel, void* b_kernel, PF_EffectWorld* dst);
    PF_Err (*copy)(void* effect_ref, PF_EffectWorld* src, PF_EffectWorld* dst, PF_LRect* src_r, PF_LRect* dst_r);
    PF_Err (*fill)(void* effect_ref, const PF_Pixel* color, const PF_LRect* dst_rect, PF_EffectWorld* world);
    PF_Err (*gaussian_kernel)(void* effect_ref, A_FpLong kRadius, A_long flags, A_FpLong multiplier, A_long* diameter, void* kernel);
    PF_Err (*iterate)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel* in, PF_Pixel* out), PF_EffectWorld* dst);
    PF_Err (*premultiply)(void* effect_ref, A_long forward, PF_EffectWorld* dst);
    PF_Err (*premultiply_color)(void* effect_ref, PF_EffectWorld* src, const PF_Pixel* color, A_long forward, PF_EffectWorld* dst);
    PF_Err (*new_world)(void* effect_ref, A_long width, A_long height, A_long flags, PF_EffectWorld* world);
    PF_Err (*dispose_world)(void* effect_ref, PF_EffectWorld* world);
    PF_Err (*iterate_origin)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, const PF_FixedPoint* origin, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel* in, PF_Pixel* out), PF_EffectWorld* dst);
    PF_Err (*iterate_LUT)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, A_u_char* a_lut0, A_u_char* r_lut0, A_u_char* g_lut0, A_u_char* b_lut0, PF_EffectWorld* dst);
    PF_Err (*transfer_rect)(void* effect_ref, A_long quality, A_long m_flags, A_long field, const PF_LRect* src_rec, const PF_EffectWorld* src_world, const void* comp_mode, const void* mask_world0, A_long dest_x, A_long dest_y, PF_EffectWorld* dst_world);
    PF_Err (*transform_world)(void* effect_ref, A_long quality, A_long m_flags, A_long field, const PF_EffectWorld* src_world, const void* comp_mode, const void* mask_world0, const void* matrices, A_long num_matrices, A_Boolean src2dst_matrix, const PF_LRect* dest_rect, PF_EffectWorld* dst_world);
    A_Handle (*host_new_handle)(A_u_int64 size);
    void* (*host_lock_handle)(A_Handle pf_handle);
    void (*host_unlock_handle)(A_Handle pf_handle);
    void (*host_dispose_handle)(A_Handle pf_handle);
    PF_Err (*get_callback_addr)(void* effect_ref, A_long quality, A_long mode_flags, A_long which_callback, void** fn_ptr);
    PF_Err (*app)(void* effect_ref, A_long, ...);
    void*  ansi;
    void*  colorCB;
    void*  get_platform_data;
    A_u_int64 (*host_get_handle_size)(A_Handle pf_handle);
    PF_Err (*iterate_origin_non_clip_src)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, const PF_FixedPoint* origin, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel* in, PF_Pixel* out), PF_EffectWorld* dst);
    PF_Err (*iterate_generic)(A_long iterationsL, void* refconPV, PF_Err (*fn_func)(void* refconPV, A_long thread_indexL, A_long i, A_long iterationsL));
    PF_Err (*subpixel_sample16)(void* effect_ref, A_Fixed x, A_Fixed y, const void* params, PF_Pixel16* dst_pixel);
    PF_Err (*area_sample16)(void* effect_ref, A_Fixed x, A_Fixed y, const void* params, PF_Pixel16* dst_pixel);
    PF_Err (*fill16)(void* effect_ref, const PF_Pixel16* color, const PF_LRect* dst_rect, PF_EffectWorld* world);
    PF_Err (*premultiply_color16)(void* effect_ref, PF_EffectWorld* src, const PF_Pixel16* color, A_long forward, PF_EffectWorld* dst);
    PF_Err (*iterate16)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel16* in, PF_Pixel16* out), PF_EffectWorld* dst);
    PF_Err (*iterate_origin16)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, const PF_FixedPoint* origin, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel16* in, PF_Pixel16* out), PF_EffectWorld* dst);
    PF_Err (*iterate_origin_non_clip_src16)(struct PF_InData* in_data, A_long progress_base, A_long progress_final, PF_EffectWorld* src, const PF_LRect* area, const PF_FixedPoint* origin, void* refcon, PF_Err (*pix_fn)(void* refcon, A_long x, A_long y, PF_Pixel16* in, PF_Pixel16* out), PF_EffectWorld* dst);
    PF_Err (*get_pixel_data8)(PF_EffectWorld* worldP, PF_PixelPtr pixelsP0, PF_Pixel8** pixPP);
    PF_Err (*get_pixel_data16)(PF_EffectWorld* worldP, PF_PixelPtr pixelsP0, PF_Pixel16** pixPP);
    A_long reserved[1];
} PF_UtilCallbacks;

/* PF_SpecVersion */
typedef struct {
    A_short         major;
    A_short         minor;
} PF_SpecVersion;

/* PF_InData & PF_OutData */
typedef struct PF_InData {
    PF_InteractCallbacks    inter;
    PF_UtilCallbacks        *utils;
    void*                   effect_ref;
    A_long                  quality;
    PF_SpecVersion          version;
    A_long                  serial_num;
    A_long                  appl_id;
    A_long                  num_params;
    A_long                  reserved;
    A_long                  what_cpu;
    A_long                  what_fpu;
    A_long                  current_time;
    A_long                  time_step;
    A_long                  total_time;
    A_long                  local_time_step;
    A_u_long                time_scale;
    A_long                  field;
    A_Fixed                 shutter_angle;
    A_long                  width;
    A_long                  height;
    PF_LRect                extent_hint;
    A_long                  output_origin_x;
    A_long                  output_origin_y;
    PF_RationalScale        downsample_x;
    PF_RationalScale        downsample_y;
    PF_RationalScale        pixel_aspect_ratio;
    A_long                  in_flags;
    A_Handle                global_data;
    A_Handle                sequence_data;
    A_Handle                frame_data;
    A_long                  start_sampL;
    A_long                  dur_sampL;
    A_long                  total_sampL;
    PF_SoundWorld           src_snd;
    struct SPBasicSuite     *pica_basicP;
    A_long                  pre_effect_source_origin_x;
    A_long                  pre_effect_source_origin_y;
    A_Fixed                 shutter_phase;
} PF_InData;

typedef struct {
    A_u_long        my_version;
    A_char          name[32];
    A_Handle        global_data;
    A_long          num_params;
    A_Handle        sequence_data;
    A_long          flat_sdata_size;
    A_Handle        frame_data;
    A_long          width;
    A_long          height;
    PF_FixedPoint   origin;
    A_long          out_flags;
    A_char          return_msg[256];
    A_long          start_sampL;
    A_long          dur_sampL;
    PF_SoundWorld   dest_snd;
    A_long          out_flags2;
} PF_OutData;

/* SmartFX Structures */
typedef enum {
    PF_PixelFormat_ARGB32 = 0,
    PF_PixelFormat_ARGB64 = 1,
    PF_PixelFormat_ARGB128 = 2,
    PF_PixelFormat_INVALID = 0x7FFFFFFF
} PF_PixelFormat;

typedef struct {
    PF_LRect        rect;
    A_long          field;
    PF_RationalScale downsample_x;
    PF_RationalScale downsample_y;
    A_long          preserve_rgb_of_zero_alpha;
    A_long          channel_mask;
    A_long          reserved[4];
} PF_RenderRequest;

typedef struct {
    PF_RenderRequest    output_request;
    A_short             bitdepth;
    const void          *pre_render_data;
    void                *pre_render_data_flat;
    A_u_long            pre_render_data_flat_size;
    A_long              reserved[4];
} PF_PreRenderInput;

typedef struct {
    PF_LRect            result_rect;
    PF_LRect            max_result_rect;
    A_Boolean           solid;
    A_Boolean           reservedB[3];
    void                *pre_render_data;
    PF_Err              (*delete_pre_render_data_func)(void* pre_render_data);
    A_long              flags;
    A_long              reserved[4];
} PF_PreRenderOutput;

typedef struct {
    PF_LRect        result_rect;
    PF_LRect        max_result_rect;
    PF_RationalScale par;
    A_long          solid;
    A_long          reserved[16];
    A_long          ref_width;
    A_long          ref_height;
} PF_CheckoutResult;

typedef struct {
    PF_Err (*checkout_layer)(
        void*                   effect_ref,
        A_long                  index,
        A_long                  checkout_idL,
        const PF_RenderRequest  *req,
        A_long                  what_time,
        A_long                  time_step,
        A_u_long                time_scale,
        PF_CheckoutResult       *checkout_result);
} PF_PreRenderCallbacks;

typedef struct {
    PF_PreRenderInput       input;
    PF_PreRenderOutput      output;
    PF_PreRenderCallbacks   *cb;
} PF_PreRenderExtra;

typedef struct {
    PF_RenderRequest    output_request;
    A_short             bitdepth;
    void                *pre_render_data;
    A_long              what_gpu;
    A_u_long            device_index;
} PF_SmartRenderInput;

typedef struct {
    PF_Err (*checkout_layer_pixels)(
        void*           effect_ref,
        A_long          checkout_idL,
        PF_EffectWorld  **pixels);
    PF_Err (*checkin_layer_pixels)(
        void*           effect_ref,
        A_long          checkout_idL);
    PF_Err (*checkout_output)(
        void*           effect_ref,
        PF_EffectWorld  **output);
} PF_SmartRenderCallbacks;

typedef struct {
    PF_SmartRenderInput     input;
    PF_SmartRenderCallbacks *cb;
} PF_SmartRenderExtra;

typedef struct {
    PF_ParamIndex   param_index;
} PF_UserChangedParamExtra;

typedef union {
    PF_PreRenderExtra           pre;
    PF_SmartRenderExtra         smart;
    PF_UserChangedParamExtra    u_change;
} PF_ExtraUnion;

#ifdef __cplusplus
}
#endif

#endif /* _H_AE_EFFECT */
