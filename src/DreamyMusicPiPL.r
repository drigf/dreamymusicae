/*******************************************************************/
/*  DreamyMusicPiPL.r                                              */
/*  Adobe After Effects PiPL Resource Definition                   */
/*******************************************************************/

#include "AE_GeneralPlug.r"

resource 'PiPL' (16000) {
    {   /* array properties: 12 elements */
        /* [1] */
        Kind {
            AEEffect
        },
        /* [2] */
        Name {
            "Dreamy Music VHS"
        },
        /* [3] */
        Category {
            "Stylize"
        },
#ifdef AE_OS_WIN
    #ifdef AE_PROC_INTELx64
        CodeWin64X86 {"EffectMain"},
    #endif
#else
    #ifdef AE_OS_MAC
        CodeMacIntel64 {"EffectMain"},
        CodeMacARM64 {"EffectMain"},
    #endif
#endif
        /* [6] */
        AE_PiPL_Version {
            2,
            0
        },
        /* [7] */
        AE_Effect_Spec_Version {
            13,
            28
        },
        /* [8] */
        AE_Effect_Version {
            524289  /* 1.0.0 Release (PF_VERSION(1,0,0,3,1)) */
        },
        /* [9] */
        AE_Effect_Info_Flags {
            0
        },
        /* [10] */
        AE_Effect_Global_OutFlags {
            0x06040404  /* PF_OutFlag_NON_PARAM_VARY | PF_OutFlag_PIX_INDEPENDENT | PF_OutFlag_I_USE_AUDIO | PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_SEND_UPDATE_PARAMS_UI */
        },
        /* [11] */
        AE_Effect_Global_OutFlags_2 {
            0x08000008  /* PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG | PF_OutFlag2_SUPPORTS_THREADED_RENDERING */
        },
        /* [12] */
        AE_Effect_Match_Name {
            "ADBE DreamyMusicVHS"
        },
        /* [13] */
        AE_Reserved_Info {
            8
        }
    }
};
