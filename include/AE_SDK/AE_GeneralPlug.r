/*******************************************************************/
/*  AE_GeneralPlug.r                                               */
/*  Rez definition for Adobe After Effects PiPL resource           */
/*******************************************************************/

#ifndef _AE_GENERAL_PLUG_R_
#define _AE_GENERAL_PLUG_R_

type 'PiPL' {
    longint = 0;
    longint = 0;
    longint = $$CountOf(properties);
    wide array properties {
        switch {
            case Kind:
                literal longint = '8BIM';
                key literal longint = 'kind';
                longint = 0;
                longint = 4;
                literal longint AEEffect = 'eFKT';

            case Name:
                literal longint = '8BIM';
                key literal longint = 'name';
                longint = 0;
                longint = (nameEnd[$$ArrayIndex(properties)] - nameStart[$$ArrayIndex(properties)]);
              nameStart:
                pstring;
              nameEnd:
                align long;

            case Category:
                literal longint = '8BIM';
                key literal longint = 'catg';
                longint = 0;
                longint = (catEnd[$$ArrayIndex(properties)] - catStart[$$ArrayIndex(properties)]);
              catStart:
                pstring;
              catEnd:
                align long;

            case CodeMacARM64:
                literal longint = '8BIM';
                key literal longint = 'mAMR';
                longint = 0;
                longint = (armEnd[$$ArrayIndex(properties)] - armStart[$$ArrayIndex(properties)]);
              armStart:
                cstring;
              armEnd:
                align long;

            case CodeMacIntel64:
                literal longint = '8BIM';
                key literal longint = '8664';
                longint = 0;
                longint = (x64End[$$ArrayIndex(properties)] - x64Start[$$ArrayIndex(properties)]);
              x64Start:
                cstring;
              x64End:
                align long;

            case CodeWin64X86:
                literal longint = '8BIM';
                key literal longint = '8664';
                longint = 0;
                longint = (win64End[$$ArrayIndex(properties)] - win64Start[$$ArrayIndex(properties)]);
              win64Start:
                cstring;
              win64End:
                align long;

            case AE_PiPL_Version:
                literal longint = '8BIM';
                key literal longint = 'ePVR';
                longint = 0;
                longint = 4;
                integer;
                integer;

            case AE_Effect_Spec_Version:
                literal longint = '8BIM';
                key literal longint = 'eSVR';
                longint = 0;
                longint = 4;
                integer;
                integer;

            case AE_Effect_Version:
                literal longint = '8BIM';
                key literal longint = 'eVER';
                longint = 0;
                longint = 4;
                longint;

            case AE_Effect_Info_Flags:
                literal longint = '8BIM';
                key literal longint = 'eINF';
                longint = 0;
                longint = 4;
                integer;
                integer = 0;

            case AE_Effect_Global_OutFlags:
                literal longint = '8BIM';
                key literal longint = 'eGLO';
                longint = 0;
                longint = 4;
                longint;

            case AE_Effect_Global_OutFlags_2:
                literal longint = '8BIM';
                key literal longint = 'eGL2';
                longint = 0;
                longint = 4;
                longint;

            case AE_Effect_Match_Name:
                literal longint = '8BIM';
                key literal longint = 'eMNA';
                longint = 0;
                longint = (matchEnd[$$ArrayIndex(properties)] - matchStart[$$ArrayIndex(properties)]);
              matchStart:
                pstring;
              matchEnd:
                align long;

            case AE_Reserved_Info:
                literal longint = '8BIM';
                key literal longint = 'aeRs';
                longint = 0;
                longint = 4;
                longint;
        };
    };
};

#endif /* _AE_GENERAL_PLUG_R_ */
