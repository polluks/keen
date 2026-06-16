#ifndef __ID_IN__
#define __ID_IN__

#include "id_heads.h"

#define MaxPlayers 4
#define MaxKbds    2
#define MaxJoys    2
#define NumCodes   128

typedef byte ScanCode;
#define sc_None       0
#define sc_Bad        0xff
#define sc_Return     0x1c
#define sc_Escape     0x01
#define sc_Space      0x39
#define sc_BackSpace  0x0e
#define sc_Tab        0x0f
#define sc_Alt        0x38
#define sc_Control    0x1d
#define sc_CapsLock   0x3a
#define sc_LShift     0x2a
#define sc_RShift     0x36
#define sc_UpArrow    0x48
#define sc_DownArrow  0x50
#define sc_LeftArrow  0x4b
#define sc_RightArrow 0x4d
#define sc_Insert     0x52
#define sc_Delete     0x53
#define sc_Home       0x47
#define sc_End        0x4f
#define sc_PgUp       0x49
#define sc_PgDn       0x51
#define sc_F1         0x3b
#define sc_F2         0x3c
#define sc_F3         0x3d
#define sc_F4         0x3e
#define sc_F5         0x3f
#define sc_F6         0x40
#define sc_F7         0x41
#define sc_F8         0x42
#define sc_F9         0x43
#define sc_F10        0x44
#define sc_F11        0x57
#define sc_F12        0x59

#define sc_A          0x1e
#define sc_B          0x30
#define sc_C          0x2e
#define sc_D          0x20
#define sc_E          0x12
#define sc_F          0x21
#define sc_G          0x22
#define sc_H          0x23
#define sc_I          0x17
#define sc_J          0x24
#define sc_K          0x25
#define sc_L          0x26
#define sc_M          0x32
#define sc_N          0x31
#define sc_O          0x18
#define sc_P          0x19
#define sc_Q          0x10
#define sc_R          0x13
#define sc_S          0x1f
#define sc_T          0x14
#define sc_U          0x16
#define sc_V          0x2f
#define sc_W          0x11
#define sc_X          0x2d
#define sc_Y          0x15
#define sc_Z          0x2c

#define key_None      0
#define key_Return    0x0d
#define key_Escape    0x1b
#define key_Space     0x20
#define key_BackSpace 0x08
#define key_Tab       0x09
#define key_Delete    0x7f

typedef enum { demo_Off, demo_Record, demo_Playback, demo_PlayDone } Demo;
typedef enum { ctrl_Keyboard, ctrl_Keyboard1 = ctrl_Keyboard, ctrl_Keyboard2,
               ctrl_Joystick, ctrl_Joystick1 = ctrl_Joystick, ctrl_Joystick2,
               ctrl_Mouse } ControlType;
typedef enum { motion_Left = -1, motion_Up = -1, motion_None = 0,
               motion_Right = 1, motion_Down = 1 } Motion;
typedef enum { dir_North, dir_NorthEast, dir_East, dir_SouthEast,
               dir_South, dir_SouthWest, dir_West, dir_NorthWest,
               dir_None } Direction;
typedef struct {
    boolean button0, button1;
    int x, y;
    Motion xaxis, yaxis;
    Direction dir;
} CursorInfo;
typedef CursorInfo ControlInfo;
typedef struct {
    ScanCode button0, button1;
    ScanCode upleft, up, upright;
    ScanCode left, right;
    ScanCode downleft, down, downright;
} KeyboardDef;
typedef struct {
    word joyMinX, joyMinY;
    word threshMinX, threshMinY;
    word threshMaxX, threshMaxY;
    word joyMaxX, joyMaxY;
    word joyMultXL, joyMultYL;
    word joyMultXH, joyMultYH;
} JoystickDef;

extern boolean Keyboard[];
extern boolean MousePresent;
extern boolean JoysPresent[];
extern Demo DemoMode;
extern boolean Paused;
extern char LastASCII;
extern ScanCode LastScan;
extern KeyboardDef KbdDefs[];
extern JoystickDef JoyDefs[];
extern ControlType Controls[MaxPlayers];

#define IN_KeyDown(code) (Keyboard[(code)])
#define IN_ClearKey(code) { Keyboard[code] = false; if (code == LastScan) LastScan = sc_None; }

extern void IN_Startup(void), IN_Shutdown(void);
extern void IN_Default(boolean gotit, ControlType in);
extern void IN_SetKeyHook(void (*)());
extern void IN_ClearKeysDown(void);
extern void IN_ReadCursor(CursorInfo *);
extern void IN_ReadControl(int, ControlInfo *);
extern void IN_SetControlType(int, ControlType);
extern void IN_GetJoyAbs(word joy, word *xp, word *yp);
extern void IN_SetupJoy(word joy, word minx, word maxx, word miny, word maxy);
extern void IN_StartDemoPlayback(byte *buffer, word bufsize);
extern void IN_StopDemo(void), IN_FreeDemoBuffer(void);
extern void IN_Ack(void), IN_AckBack(void);
extern boolean IN_UserInput(longword delay, boolean clear);
extern boolean IN_IsUserInput(void);
extern boolean IN_StartDemoRecord(word bufsize);
extern byte *IN_GetScanName(ScanCode);
extern char IN_WaitForASCII(void);
extern ScanCode IN_WaitForKey(void);
extern word IN_GetJoyButtonsDB(word joy);

#endif
