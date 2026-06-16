#include "id_heads.h"
#include <stdio.h>
#include <string.h>

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
