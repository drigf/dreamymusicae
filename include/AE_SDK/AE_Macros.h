/*******************************************************************/
/*  AE_Macros.h & Parameter Addition Macros                        */
/*******************************************************************/

#ifndef _H_AE_MACROS
#define _H_AE_MACROS

#include "AE_Effect.h"
#include <string.h>

#ifndef AEFX_CLR_STRUCT
#define AEFX_CLR_STRUCT(STRUCT) memset(&(STRUCT), 0, sizeof(STRUCT))
#endif

#define PF_ADD_PARAM(IN_DATA, INDEX, DEF) \
    (*(IN_DATA)->inter.add_param)((IN_DATA)->effect_ref, (INDEX), (DEF))

#define PF_CHECKOUT_PARAM(IN_DATA, INDEX, TIME, STEP, SCALE, PARAM) \
    (*(IN_DATA)->inter.checkout_param)((IN_DATA)->effect_ref, (INDEX), (TIME), (STEP), (SCALE), (PARAM))

#define PF_CHECKIN_PARAM(IN_DATA, PARAM) \
    (*(IN_DATA)->inter.checkin_param)((IN_DATA)->effect_ref, (PARAM))

#define PF_CHECKOUT_LAYER_AUDIO(IN_DATA, INDEX, START, DUR, SCALE, RATE, BYTES, CHANS, FMT, AUDIO) \
    (*(IN_DATA)->inter.checkout_layer_audio)((IN_DATA)->effect_ref, (INDEX), (START), (DUR), (SCALE), (RATE), (BYTES), (CHANS), (FMT), (AUDIO))

#define PF_CHECKIN_LAYER_AUDIO(IN_DATA, AUDIO) \
    (*(IN_DATA)->inter.checkin_layer_audio)((IN_DATA)->effect_ref, (AUDIO))

#define PF_GET_AUDIO_DATA(IN_DATA, AUDIO, DATA, SAMPLES, RATE, BYTES, CHANS, FMT) \
    (*(IN_DATA)->inter.get_audio_data)((IN_DATA)->effect_ref, (AUDIO), (DATA), (SAMPLES), (RATE), (BYTES), (CHANS), (FMT))

#define PF_ABORT(IN_DATA) \
    (*(IN_DATA)->inter.abort)((IN_DATA)->effect_ref)

#define PF_PROGRESS(IN_DATA, CURRENT, TOTAL) \
    (*(IN_DATA)->inter.progress)((IN_DATA)->effect_ref, (CURRENT), (TOTAL))

#define PF_COPY(SRC, DST, SRC_RECT, DST_RECT) \
    (*(in_data->utils->copy))(in_data->effect_ref, (SRC), (DST), (SRC_RECT), (DST_RECT))

#define PF_ITERATE_GENERIC(ITERATIONS, REFCON, FN) \
    (*(in_data->utils->iterate_generic))((ITERATIONS), (REFCON), (FN))

/* Parameter setup helper macros */

#define PF_ADD_TOPIC(NAME, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_GROUP_START; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_END_TOPIC(ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_GROUP_END; \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_ADD_FLOAT_SLIDERX(NAME, VALID_MIN, VALID_MAX, SLIDER_MIN, SLIDER_MAX, DFLT, PREC, DISP, FLAGS, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_FLOAT_SLIDER; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.flags = (FLAGS); \
        def.u.fs_d.valid_min = (A_FpLong)(VALID_MIN); \
        def.u.fs_d.slider_min = (A_FpLong)(SLIDER_MIN); \
        def.u.fs_d.valid_max = (A_FpLong)(VALID_MAX); \
        def.u.fs_d.slider_max = (A_FpLong)(SLIDER_MAX); \
        def.u.fs_d.value = (A_FpLong)(DFLT); \
        def.u.fs_d.dephault = (A_FpLong)(DFLT); \
        def.u.fs_d.precision = (A_short)(PREC); \
        def.u.fs_d.display_flags = (A_short)(DISP); \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_ADD_CHECKBOXX(NAME, DFLT, FLAGS, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_CHECKBOX; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.flags = (FLAGS); \
        def.u.bd.value = (DFLT); \
        def.u.bd.dephault = (A_Boolean)(DFLT); \
        def.u.bd.u.nameptr = ""; \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_ADD_COLOR(NAME, R, G, B, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_COLOR; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.u.cd.dephault.alpha = 255; \
        def.u.cd.dephault.red = (A_u_char)(R); \
        def.u.cd.dephault.green = (A_u_char)(G); \
        def.u.cd.dephault.blue = (A_u_char)(B); \
        def.u.cd.value = def.u.cd.dephault; \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_ADD_POPUPX(NAME, CHOICES_COUNT, DFLT, CHOICES_STR, FLAGS, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_POPUP; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.flags = (FLAGS); \
        def.u.pd.num_choices = (A_short)(CHOICES_COUNT); \
        def.u.pd.dephault = (A_short)(DFLT); \
        def.u.pd.value = (DFLT); \
        def.u.pd.u.namesptr = (CHOICES_STR); \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#define PF_ADD_LAYER(NAME, DFLT, ID) \
    do { \
        PF_Err priv_err = PF_Err_NONE; \
        def.param_type = PF_Param_LAYER; \
        strncpy(def.name, (NAME), 31); \
        def.name[31] = '\0'; \
        def.u.layer_d.dephault = (DFLT); \
        def.uu.id = (ID); \
        if ((priv_err = PF_ADD_PARAM(in_data, -1, &def)) != PF_Err_NONE) return priv_err; \
    } while (0)

#endif /* _H_AE_MACROS */
