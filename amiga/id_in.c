#include "id_heads.h"
#include "rtg_driver.h"
#include <devices/inputevent.h>
#include <intuition/intuition.h>

boolean Keyboard[NumCodes];
boolean JoysPresent[MaxJoys];
boolean MousePresent;
Demo DemoMode = demo_Off;
boolean Paused;
char LastASCII;
ScanCode LastScan;
KeyboardDef KbdDefs[MaxKbds] = {
    {0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51}
};
JoystickDef JoyDefs[MaxJoys];
ControlType Controls[MaxPlayers];

static boolean IN_Started;
static boolean CapsLock;
static ScanCode CurCode, LastCode;

static byte ASCIINames[] = {
    0, 27,'1','2','3','4','5','6','7','8','9','0','-','=',8, 9,
    'q','w','e','r','t','y','u','i','o','p','[',']',13, 0 ,'a','s',
    'd','f','g','h','j','k','l',';',39, '`', 0, 92,'z','x','c','v',
    'b','n','m',',','.','/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, '7','8','9','-','4','5','6','+','1',
    '2','3','0',127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static byte ShiftNames[] = {
    0, 27,'!','@','#','$','%','^','&','*','(',')','_','+',8, 9,
    'Q','W','E','R','T','Y','U','I','O','P','{','}',13, 0 ,'A','S',
    'D','F','G','H','J','K','L',':',34, '~', 0,'|','Z','X','C','V',
    'B','N','M','<','>','?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, '7','8','9','-','4','5','6','+','1',
    '2','3','0',127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned short amiga_to_pc_scancode(UWORD code)
{
    switch (code) {
        case 0x00: return sc_BackSpace;
        case 0x01: return sc_Tab;
        case 0x03: return sc_Return;
        case 0x04: return sc_Return;
        case 0x0C: return sc_Space;
        case 0x0F: return sc_Escape;
        case 0x10: return sc_F1;
        case 0x11: return sc_F2;
        case 0x12: return sc_F3;
        case 0x13: return sc_F4;
        case 0x14: return sc_F5;
        case 0x15: return sc_F6;
        case 0x16: return sc_F7;
        case 0x17: return sc_F8;
        case 0x18: return sc_F9;
        case 0x19: return sc_F10;
        case 0x1A: return sc_F11;
        case 0x1B: return sc_F12;
        case 0x1C: return sc_Home;
        case 0x1D: return sc_UpArrow;
        case 0x1E: return sc_PgUp;
        case 0x1F: return sc_LeftArrow;
        case 0x20: return sc_RightArrow;
        case 0x21: return sc_End;
        case 0x22: return sc_DownArrow;
        case 0x23: return sc_PgDn;
        case 0x24: return sc_Insert;
        case 0x25: return sc_Delete;
        case 0x2C: return sc_Space;
        case 0x32: return sc_UpArrow;
        case 0x33: return sc_DownArrow;
        case 0x34: return sc_LeftArrow;
        case 0x35: return sc_RightArrow;
        case 0x36: return sc_Return;
        case 0x3F: return sc_LShift;
        case 0x40: return sc_RShift;
        case 0x41: return sc_CapsLock;
        case 0x42: return sc_Control;
        case 0x43: return sc_Alt;
        case 0x44: return sc_Alt;
        case 0x45: return sc_Control;
        case 0x46: return sc_LShift;
        case 0x47: return sc_RShift;
        case 0x60: return sc_BackSpace;
        case 0x61: return sc_Return;
        case 0x62: return sc_Escape;
        case 0x63: return sc_Delete;
        case 0x64: return sc_Tab;
        default:
            if (code >= 0x20 && code <= 0x39)
                return code - 0x20 + 0x10;
            return sc_None;
    }
}

void IN_Startup(void)
{
    int i;
    for (i = 0; i < NumCodes; i++)
        Keyboard[i] = false;
    LastASCII = 0;
    LastScan = sc_None;
    IN_Started = true;
    Paused = false;
    DemoMode = demo_Off;
    Controls[0] = ctrl_Keyboard1;
    Controls[1] = ctrl_Keyboard2;
    MousePresent = false;
    JoysPresent[0] = false;
    JoysPresent[1] = false;
}

void IN_Shutdown(void)
{
    IN_Started = false;
}

void IN_Default(boolean gotit, ControlType in) {}
void IN_SetKeyHook(void (*hook)()) {}

void IN_ClearKeysDown(void)
{
    int i;
    for (i = 0; i < NumCodes; i++)
        Keyboard[i] = false;
    LastASCII = 0;
    LastScan = sc_None;
}

void IN_ReadCursor(CursorInfo *ci)
{
    ci->x = 0;
    ci->y = 0;
    ci->button0 = false;
    ci->button1 = false;
    ci->xaxis = motion_None;
    ci->yaxis = motion_None;
    ci->dir = dir_None;

    if (Keyboard[sc_LeftArrow] || Keyboard[sc_A]) {
        ci->xaxis = motion_Left;
        ci->x = -1;
    } else if (Keyboard[sc_RightArrow] || Keyboard[sc_D]) {
        ci->xaxis = motion_Right;
        ci->x = 1;
    }

    if (Keyboard[sc_UpArrow] || Keyboard[sc_W]) {
        ci->yaxis = motion_Up;
        ci->y = -1;
    } else if (Keyboard[sc_DownArrow] || Keyboard[sc_S]) {
        ci->yaxis = motion_Down;
        ci->y = 1;
    }

    if (ci->xaxis == motion_Left && ci->yaxis == motion_Up)
        ci->dir = dir_NorthWest;
    else if (ci->xaxis == motion_Left && ci->yaxis == motion_Down)
        ci->dir = dir_SouthWest;
    else if (ci->xaxis == motion_Right && ci->yaxis == motion_Up)
        ci->dir = dir_NorthEast;
    else if (ci->xaxis == motion_Right && ci->yaxis == motion_Down)
        ci->dir = dir_SouthEast;
    else if (ci->xaxis == motion_Left) ci->dir = dir_West;
    else if (ci->xaxis == motion_Right) ci->dir = dir_East;
    else if (ci->yaxis == motion_Up) ci->dir = dir_North;
    else if (ci->yaxis == motion_Down) ci->dir = dir_South;
    else ci->dir = dir_None;

    ci->button0 = (Keyboard[sc_Control] || Keyboard[sc_Space] || Keyboard[sc_Return]);
    ci->button1 = (Keyboard[sc_Alt] || Keyboard[sc_Escape]);
}

void IN_ReadControl(int player, ControlInfo *ci)
{
    IN_ReadCursor((CursorInfo *)ci);
}

void IN_SetControlType(int player, ControlType type)
{
    if (player >= 0 && player < MaxPlayers)
        Controls[player] = type;
}

void IN_GetJoyAbs(word joy, word *xp, word *yp)
{
    *xp = 0;
    *yp = 0;
}

void IN_SetupJoy(word joy, word minx, word maxx, word miny, word maxy) {}

void IN_StartDemoPlayback(byte *buffer, word bufsize)
{
    DemoMode = demo_Playback;
}

void IN_StopDemo(void)
{
    DemoMode = demo_Off;
}

void IN_FreeDemoBuffer(void) {}

void IN_Ack(void)
{
    IN_ClearKeysDown();
    while (!Keyboard[sc_Space] && !Keyboard[sc_Return] && !Keyboard[sc_Control]) {
        VW_UpdateScreen();
    }
    IN_ClearKeysDown();
}

void IN_AckBack(void)
{
    IN_Ack();
}

boolean IN_UserInput(longword delay, boolean clear)
{
    if (clear)
        IN_ClearKeysDown();
    if (LastScan != sc_None)
        return true;
    return false;
}

boolean IN_IsUserInput(void)
{
    return (LastScan != sc_None);
}

boolean IN_StartDemoRecord(word bufsize)
{
    return false;
}

byte *IN_GetScanName(ScanCode sc)
{
    static char buf[8];
    sprintf(buf, "%02x", sc);
    return (byte *)buf;
}

char IN_WaitForASCII(void)
{
    while (LastASCII == 0) {
        VW_UpdateScreen();
    }
    {
        char c = LastASCII;
        LastASCII = 0;
        return c;
    }
}

ScanCode IN_WaitForKey(void)
{
    while (LastScan == sc_None) {
        VW_UpdateScreen();
    }
    {
        ScanCode sc = LastScan;
        LastScan = sc_None;
        return sc;
    }
}

word IN_GetJoyButtonsDB(word joy)
{
    return 0;
}

void IN_ProcessEvent(UWORD code, int pressed)
{
    ScanCode sc = amiga_to_pc_scancode(code);

    if (sc == sc_None) return;

    if (pressed) {
        if (!Keyboard[sc]) {
            Keyboard[sc] = true;
            LastScan = sc;

            if (sc >= sc_A && sc <= sc_Z) {
                int idx = sc - sc_A;
                LastASCII = (Keyboard[sc_LShift] || Keyboard[sc_RShift])
                    ? 'A' + idx : 'a' + idx;
            } else {
                int i;
                for (i = 0; i < 128; i++) {
                    if (ASCIINames[i] && i == sc) {
                        LastASCII = (Keyboard[sc_LShift] || Keyboard[sc_RShift])
                            ? ShiftNames[i] : ASCIINames[i];
                        break;
                    }
                }
            }
        }
    } else {
        Keyboard[sc] = false;
        if (sc == LastScan)
            LastScan = sc_None;
    }
}
