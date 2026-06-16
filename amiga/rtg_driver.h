#ifndef __RTG_DRIVER__
#define __RTG_DRIVER__

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>

#define RTG_DISPLAY_WIDTH  640
#define RTG_DISPLAY_HEIGHT 400
#define RTG_DISPLAY_DEPTH  8

#define VIRTUAL_WIDTH  512
#define VIRTUAL_HEIGHT 300
#define SCREEN_WIDTH   64
#define LINE_WIDTH     64

typedef struct {
    UBYTE *framebuffer;
    UWORD width;
    UWORD height;
    UBYTE depth;
    UWORD bytes_per_row;
    struct Window *window;
    struct Screen *screen;
    UBYTE palette[256*3];
} RTGDevice;

extern RTGDevice rtg;

int  RTG_OpenScreen(void);
void RTG_CloseScreen(void);
void RTG_SetPalette(UBYTE *pal, UWORD first, UWORD count);
void RTG_Clear(UBYTE color);
void RTG_SetPixel(UWORD x, UWORD y, UBYTE color);
void RTG_DrawHLine(UWORD x1, UWORD x2, UWORD y, UBYTE color);
void RTG_DrawVLine(UWORD y1, UWORD y2, UWORD x, UBYTE color);
void RTG_FillRect(UWORD x, UWORD y, UWORD w, UWORD h, UBYTE color);
void RTG_Blit(UBYTE *src, UWORD src_x, UWORD src_y, UWORD src_bpr,
              UWORD dst_x, UWORD dst_y, UWORD w, UWORD h);
void RTG_BlitMasked(UBYTE *src, UWORD src_x, UWORD src_y, UWORD src_bpr,
                    UWORD dst_x, UWORD dst_y, UWORD w, UWORD h,
                    UBYTE transparent);
void RTG_Refresh(void);
void RTG_WaitVBlank(void);

#endif
