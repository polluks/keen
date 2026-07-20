#include "id_heads.h"
#include <stdio.h>
#include <string.h>
#include <dos/dos.h>

#define BORDERCOLOR SECONDCOLOR

int _argc = 0;
char **_argv = NULL;

void US_Startup(void)
{
}

void US_Shutdown(void)
{
}

int US_CheckParm(char *parm, char *strings[])
{
    int i;
    for (i = 0; strings[i] && strings[i][0]; i++) {
        if (strcmp(parm, strings[i]) == 0)
            return i;
    }
    return -1;
}

void US_Print(char *s)
{
    px = 0;
    VW_DrawPropString(s);
    VW_UpdateScreen();
}

void US_CPrint(char *s)
{
    word w, h;
    VW_MeasurePropString(s, &w, &h);
    px = (VIRTUALWIDTH - (int)w) / 2;
    VW_DrawPropString(s);
}

void US_PrintUnsigned(longword n)
{
    char buf[16];
    sprintf(buf, "%lu", n);
    VW_DrawPropString(buf);
}

void US_PrintSigned(long n)
{
    char buf[16];
    sprintf(buf, "%ld", n);
    VW_DrawPropString(buf);
}

void US_PrintBig(char *s)
{
    US_Print(s);
}

static int win_x, win_y, win_w, win_h;

void US_CenterWindow(int w, int h)
{
    win_w = w * 8;
    win_h = h * 8;
    win_x = (VIRTUALWIDTH - win_w) / 2;
    win_y = (VIRTUALHEIGHT - win_h) / 2;
    US_DrawWindow(win_x, win_y, win_w, win_h);
    px = win_x + 8;
    py = win_y + 8;
    fontcolor = WHITE;
}

void US_ExitWindow(void)
{
    VW_UpdateScreen();
}

void US_ClearWindow(void)
{
    VW_Bar(win_x, win_y, win_w, win_h, BLACK);
}

void US_DrawWindow(int x, int y, int w, int h)
{
    VW_Bar(x, y, w, h, BORDERCOLOR);
    VW_Bar(x + 1, y + 1, w - 2, h - 2, BLACK);
}

void US_UpdateCursor(int x, int y)
{
    px = x;
    py = y;
}

/*
 * Random number generator (ported from id_us_a.asm)
 * Table-driven PRNG by John Carmack.
 */
static unsigned rndindex;

static const unsigned char rndtable[256] = {
      0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
     74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
     95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
     52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
     25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
     94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
     80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
     24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
     28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
     71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
     17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
    120, 163, 236, 249
};

void US_InitRndT(boolean randomize)
{
    if (randomize) {
        struct DateStamp ds;
        DateStamp(&ds);
        rndindex = ds.ds_Tick;
    } else {
        rndindex = 0;
    }
}

int US_RndT(void)
{
    rndindex = (rndindex + 1) & 0xff;
    return rndtable[rndindex];
}
