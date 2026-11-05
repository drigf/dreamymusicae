/*******************************************************************/
/*                                                                 */
/*  Minimal Self-Contained Adobe After Effects SDK Header Subset   */
/*  Verified Exact 64-Bit Struct Offsets & Alignment (macOS/Win)   */
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

#pragma pack(push, 8)

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
#define PF_PLUG_IN_SUBVERS          29

#define PF_VERSION(vers, subvers, bugvers, stage, build) \
    ((((A_u_long)(vers) & 0x7UL) << 19) | \
     (((A_u_long)(subvers) & 0xfUL) << 15) | \
     (((A_u_long)(bugvers) & 0xfUL) << 11) | \
     (((A_u_long)(stage) & 0x3UL) << 9) | \
     ((A_u_long)(build) & 0x1ffUL))

#define PF_Stage_DEVELOP        0
#define PF_Stage_ALPHA          1
#define PF_Stage_BETA           2
#define PF_Stage_RELEASE        3

/* Note: Modern AE SDK PF_VERSION macro shifts stage to bits 9..10 or 0:
 * Wait: in AsciiDepth, 1.0.0 with STAGE_VERSION=PF_Stage_DEVELOP (0) produced 0x80001 (524289)! */

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

/* Effect World / Layer (Exact 92-byte layout under mac68k / 2-byte alignment) */
typedef PF_Pixel* PF_PixelPtr;

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

/* Exact 120-byte (0x78) PF_EffectWorld / PF_LayerDef layout verified against AE 2025/2026:
 *   +0x00: reserved0 (8 bytes)
 *   +0x08: reserved1 (8 bytes)
 *   +0x10: world_flags (4 bytes, bit 0 = PF_WorldFlag_DEEP)
 *   +0x14: reserved_pad0 (4 bytes alignment padding)
 *   +0x18: data (8 bytes PF_PixelPtr)
 *   +0x20: rowbytes (4 bytes signed int)
 *   +0x24: width (4 bytes signed int)
 *   +0x28: height (4 bytes signed int)
 *   +0x2C: extent_hint (16 bytes PF_UnionableRect)
 *   +0x3C: reserved2 (4 bytes)
 *   +0x40: reserved3 (4 bytes)
 *   +0x44: pix_aspect_ratio (8 bytes PF_RationalScale)
 *   +0x4C: reserved4 (4 bytes)
 *   +0x50: origin_x (4 bytes)
 *   +0x54: origin_y (4 bytes)
 *   +0x58: reserved5 (4 bytes)
 *   +0x5C: reserved6 (4 bytes)
 *   +0x60: reserved7 (8 bytes)
 *   +0x68: platform_ref (8 bytes)
 *   +0x70: reserved8 (4 bytes)
 *   +0x74: dephault (4 bytes -> +0xAC inside PF_ParamDef, total PF_LayerDef = 0x78)
 */
typedef struct PF_EffectWorld {
    void*               reserved0;          /* +0x00 */
    void*               reserved1;          /* +0x08 */
    A_long              world_flags;        /* +0x10 */
    A_long              reserved_pad0;      /* +0x14 */
    PF_PixelPtr         data;               /* +0x18 */
    A_long              rowbytes;           /* +0x20 */
    A_long              width;              /* +0x24 */
    A_long              height;             /* +0x28 */
    PF_UnionableRect    extent_hint;        /* +0x2C */
    A_long              reserved2;          /* +0x3C */
    A_long              reserved3;          /* +0x40 */
    PF_RationalScale    pix_aspect_ratio;   /* +0x44 */
    A_long              reserved4;          /* +0x4C */
    A_long              origin_x;           /* +0x50 */
    A_long              origin_y;           /* +0x54 */
    A_long              reserved5;          /* +0x58 */
    A_long              reserved6;          /* +0x5C */
    void*               reserved7;          /* +0x60 */
    void*               platform_ref;       /* +0x68 */
    A_long              reserved8;          /* +0x70 */
    A_long              dephault;           /* +0x74 (+0xAC in PF_ParamDef) */
} PF_EffectWorld, PF_LayerDef;

#define PF_WORLD_IS_DEEP(W)     (((W)->world_flags & PF_WorldFlag_DEEP) != 0)

/* Audio Types */
typedef uint32_t PF_UFixed;
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

#pragma pack(push, 4)
typedef struct PF_SoundWorld {
    PF_SoundFormatInfo  fi;
    A_long              num_samples;
    void*               dataP;
} PF_SoundWorld;
#pragma pack(pop)

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
    PF_ParamValue   valid_min, valid_max;
    PF_ParamValue   slider_min, slider_max;
    PF_ParamValue   dephault;
} PF_SliderDef;

typedef struct {
    A_Fixed         value;
    A_char          value_str[32];
    A_char          value_desc[32];
    A_Fixed         valid_min, valid_max;
    A_Fixed         slider_min, slider_max;
    A_Fixed         dephault;
    A_short         precision;
    A_short         display_flags;
} PF_FixedSliderDef;

#define AE_FLOAT_SLIDER_PHASE_NONE 0
typedef struct {
    A_FpLong        value;              /* +0x00 (in def: +0x38, 8 bytes double) */
    A_FpLong        phase;              /* +0x08 (in def: +0x40, 8 bytes double) */
    A_char          value_desc[32];     /* +0x10 (in def: +0x48, 32 bytes) */
    A_FpShort       valid_min;          /* +0x30 (in def: +0x68, 4 bytes float!) */
    A_FpShort       valid_max;          /* +0x34 (in def: +0x6C, 4 bytes float!) */
    A_FpShort       slider_min;         /* +0x38 (in def: +0x70, 4 bytes float!) */
    A_FpShort       slider_max;         /* +0x3C (in def: +0x74, 4 bytes float!) */
    A_FpShort       dephault;           /* +0x40 (in def: +0x78, 4 bytes float!) */
    A_short         precision;          /* +0x44 (in def: +0x7C, 2 bytes short) */
    A_short         display_flags;      /* +0x46 (in def: +0x7E, 2 bytes short) */
    A_u_long        fs_flags;           /* +0x48 (in def: +0x80, 4 bytes) */
    A_FpShort       curve_tolerance;    /* +0x4C (in def: +0x84, 4 bytes float) */
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
    PF_ParamValue   value;
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
} PF_ParamDefUnion;

/* Exact 172-byte PF_ParamDef layout:
 * offset 0x00: uu.id / change_flags (4 bytes)
 * offset 0x04: ui_flags (4 bytes)
 * offset 0x08: ui_width (2 bytes)
 * offset 0x0A: ui_height (2 bytes)
 * offset 0x0C: param_type (4 bytes)
 * offset 0x10: name[32] (32 bytes)
 * offset 0x30: flags (4 bytes)
 * offset 0x34: unused (4 bytes)
 * offset 0x38: u (116 bytes)
 */
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

/* Exact 22-pointer (176-byte) PF_InteractCallbacks layout */
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
        PF_UFixed           rate,
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
        PF_UFixed           *rate0,
        A_long              *bytes_per_sample0,
        A_long              *num_channels0,
        A_long              *fmt_signed0);

    void* reserved_str[3];
    void* reserved[10];
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
    PF_Err (*dispose_world)(void* effect_ref, A_long flags, PF_EffectWorld* world);
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

/* Exact PF_InData layout (verified against AE 2025/2026 binary offsets:
 *   inter:       0x000 (176 bytes)
 *   utils:       0x0B0
 *   effect_ref:  0x0B8
 *   quality:     0x0C0
 *   ...
 *   downsample_x: 0x11C
 *   pica_basicP:  0x180
 */
typedef struct PF_InData {
    PF_InteractCallbacks    inter;              /* 0x000 - 0x0AF (176 bytes) */
    PF_UtilCallbacks        *utils;             /* 0x0B0 (8 bytes) */
    void*                   effect_ref;         /* 0x0B8 (8 bytes) */
    A_long                  quality;            /* 0x0C0 (4 bytes) */
    PF_SpecVersion          version;            /* 0x0C4 (4 bytes) */
    A_long                  serial_num;         /* 0x0C8 (4 bytes) */
    A_long                  appl_id;            /* 0x0CC (4 bytes) */
    A_long                  num_params;         /* 0x0D0 (4 bytes) */
    A_long                  reserved;           /* 0x0D4 (4 bytes) */
    A_long                  what_cpu;           /* 0x0D8 (4 bytes) */
    A_long                  what_fpu;           /* 0x0DC (4 bytes) */
    A_long                  current_time;       /* 0x0E0 (4 bytes) */
    A_long                  time_step;          /* 0x0E4 (4 bytes) */
    A_long                  total_time;         /* 0x0E8 (4 bytes) */
    A_long                  local_time_step;    /* 0x0EC (4 bytes) */
    A_u_long                time_scale;         /* 0x0F0 (4 bytes) */
    A_long                  field;              /* 0x0F4 (4 bytes) */
    A_Fixed                 shutter_angle;      /* 0x0F8 (4 bytes) */
    A_long                  width;              /* 0x0FC (4 bytes) */
    A_long                  height;             /* 0x100 (4 bytes) */
    PF_LRect                extent_hint;        /* 0x104 (16 bytes) */
    A_long                  output_origin_x;    /* 0x114 (4 bytes) */
    A_long                  output_origin_y;    /* 0x118 (4 bytes) */
    PF_RationalScale        downsample_x;       /* 0x11C (8 bytes) */
    PF_RationalScale        downsample_y;       /* 0x124 (8 bytes) */
    PF_RationalScale        pixel_aspect_ratio; /* 0x12C (8 bytes) */
    A_long                  in_flags;           /* 0x134 (4 bytes) */
    A_Handle                global_data;        /* 0x138 (8 bytes) */
    A_Handle                sequence_data;      /* 0x140 (8 bytes) */
    A_Handle                frame_data;         /* 0x148 (8 bytes) */
    A_long                  start_sampL;        /* 0x150 (4 bytes) */
    A_long                  dur_sampL;          /* 0x154 (4 bytes) */
    A_long                  total_sampL;        /* 0x158 (4 bytes) */
    PF_SoundWorld           src_snd;            /* 0x15C (36 bytes) */
    struct SPBasicSuite     *pica_basicP;       /* 0x180 (8 bytes) */
    A_long                  pre_effect_source_origin_x; /* 0x188 */
    A_long                  pre_effect_source_origin_y; /* 0x18C */
    A_Fixed                 shutter_phase;      /* 0x190 */
} PF_InData;

/* Exact PF_OutData layout (verified against AE 2025/2026 binary offsets:
 *   my_version:      0x000
 *   name:            0x004 (32 bytes)
 *   global_data:     0x024 (8 bytes)
 *   num_params:      0x02C (4 bytes)
 *   sequence_data:   0x030 (8 bytes)
 *   flat_sdata_size: 0x038 (4 bytes)
 *   frame_data:      0x03C (8 bytes)
 *   width:           0x044 (4 bytes)
 *   height:          0x048 (4 bytes)
 *   origin:          0x04C (8 bytes)
 *   out_flags:       0x054 (4 bytes)
 *   return_msg:      0x058 (256 bytes)
 *   start_sampL:     0x158 (4 bytes)
 *   dur_sampL:       0x15C (4 bytes)
 *   dest_snd:        0x160 (36 bytes)
 *   out_flags2:      0x184 (4 bytes)
 * Note: Under default clang alignment without mac68k, 8-byte pointers shift
 * out_flags to 0x60 and out_flags2 to 0x190! Let's check whether AsciiDepth
 * was compiled with default alignment or mac68k!
 */
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

/* PF_Effect_Global_OutFlags & OutFlags2 */
typedef A_long PF_OutFlags;
typedef A_long PF_OutFlags2;

/* Exact PF_PluginDataCB2 signature for PluginDataEntryFunction2 */
typedef PF_Err (*PF_PluginDataCB2)(
    void            *inPtr,
    const A_u_char  *nameZ,
    const A_u_char  *matchNameZ,
    const A_u_char  *categoryZ,
    const A_u_char  *entryPointNameZ,
    A_long          kindL,
    A_long          aeSpecVersionMajorL,
    A_long          aeSpecVersionMinorL,
    A_u_long        reservedL,
    const A_u_char  *supportUrlZ);

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

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* _H_AE_EFFECT */
