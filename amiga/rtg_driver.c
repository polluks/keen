#include <exec/exec.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>

#include "rtg_driver.h"

RTGDevice rtg;

static struct Library *IntuitionBase;
static struct IntuitionBase *Intuition;
static struct GfxBase *Gfx;
static struct MsgPort *timer_port;
static struct timerequest *timer_io;

int RTG_OpenScreen(void)
{
    UWORD i;

    IntuitionBase = OpenLibrary("intuition.library", 37);
    if (!IntuitionBase)
        return 0;
    Intuition = (struct IntuitionBase *)IntuitionBase;

    Gfx = (struct GfxBase *)OpenLibrary("graphics.library", 37);
    if (!Gfx) {
        CloseLibrary(IntuitionBase);
        return 0;
    }

    rtg.window = OpenWindowTags(NULL,
        WA_Left,        0,
        WA_Top,         0,
        WA_Width,       RTG_DISPLAY_WIDTH,
        WA_Height,      RTG_DISPLAY_HEIGHT,
        WA_Flags,       WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE,
        WA_IDCMP,       IDCMP_VANILLAKEY | IDCMP_CLOSEWINDOW | IDCMP_RAWKEY,
        WA_Title,       (ULONG)"Commander Keen - Amiga RTG",
        TAG_DONE);

    if (!rtg.window) {
        CloseLibrary((struct Library *)Gfx);
        CloseLibrary(IntuitionBase);
        return 0;
    }

    rtg.width = RTG_DISPLAY_WIDTH;
    rtg.height = RTG_DISPLAY_HEIGHT;
    rtg.depth = RTG_DISPLAY_DEPTH;
    rtg.bytes_per_row = RTG_DISPLAY_WIDTH;
    rtg.framebuffer = AllocVec(rtg.width * rtg.height, MEMF_CLEAR);
    if (!rtg.framebuffer) {
        CloseWindow(rtg.window);
        CloseLibrary((struct Library *)Gfx);
        CloseLibrary(IntuitionBase);
        return 0;
    }

    rtg.screen = rtg.window->WScreen;

    for (i = 0; i < 256*3; i++)
        rtg.palette[i] = 0;

    timer_port = CreateMsgPort();
    if (timer_port) {
        timer_io = (struct timerequest *)CreateIORequest(timer_port,
                        sizeof(struct timerequest));
        if (timer_io) {
            if (OpenDevice("timer.device", UNIT_VBLANK,
                (struct IORequest *)timer_io, 0)) {
                DeleteIORequest((struct IORequest *)timer_io);
                timer_io = NULL;
            }
        }
    }

    return 1;
}

void RTG_CloseScreen(void)
{
    if (timer_io) {
        CloseDevice((struct IORequest *)timer_io);
        DeleteIORequest((struct IORequest *)timer_io);
    }
    if (timer_port)
        DeleteMsgPort(timer_port);

    if (rtg.framebuffer)
        FreeVec(rtg.framebuffer);
    if (rtg.window)
        CloseWindow(rtg.window);
    if (Gfx)
        CloseLibrary((struct Library *)Gfx);
    if (Intuition)
        CloseLibrary(IntuitionBase);

    rtg.framebuffer = NULL;
    rtg.window = NULL;
    rtg.screen = NULL;
}

void RTG_SetPalette(UBYTE *pal, UWORD first, UWORD count)
{
    UWORD i;
    for (i = 0; i < count && (first + i) < 256; i++) {
        rtg.palette[(first + i) * 3 + 0] = pal[i * 3 + 0];
        rtg.palette[(first + i) * 3 + 1] = pal[i * 3 + 1];
        rtg.palette[(first + i) * 3 + 2] = pal[i * 3 + 2];
    }
}

void RTG_Clear(UBYTE color)
{
    memset(rtg.framebuffer, color, rtg.width * rtg.height);
}

void RTG_SetPixel(UWORD x, UWORD y, UBYTE color)
{
    if (x < rtg.width && y < rtg.height)
        rtg.framebuffer[y * rtg.bytes_per_row + x] = color;
}

void RTG_DrawHLine(UWORD x1, UWORD x2, UWORD y, UBYTE color)
{
    UWORD x;
    UBYTE *p;
    if (y >= rtg.height) return;
    if (x1 > x2) { UWORD t = x1; x1 = x2; x2 = t; }
    if (x1 >= rtg.width) return;
    if (x2 >= rtg.width) x2 = rtg.width - 1;
    p = rtg.framebuffer + y * rtg.bytes_per_row + x1;
    for (x = x1; x <= x2; x++)
        *p++ = color;
}

void RTG_DrawVLine(UWORD y1, UWORD y2, UWORD x, UBYTE color)
{
    UWORD y;
    if (x >= rtg.width) return;
    if (y1 > y2) { UWORD t = y1; y1 = y2; y2 = t; }
    if (y1 >= rtg.height) return;
    if (y2 >= rtg.height) y2 = rtg.height - 1;
    for (y = y1; y <= y2; y++)
        rtg.framebuffer[y * rtg.bytes_per_row + x] = color;
}

void RTG_FillRect(UWORD x, UWORD y, UWORD w, UWORD h, UBYTE color)
{
    UWORD ix, iy;
    UBYTE *p;
    for (iy = 0; iy < h && (y + iy) < rtg.height; iy++) {
        p = rtg.framebuffer + (y + iy) * rtg.bytes_per_row + x;
        for (ix = 0; ix < w && (x + ix) < rtg.width; ix++)
            p[ix] = color;
    }
}

void RTG_Blit(UBYTE *src, UWORD src_x, UWORD src_y, UWORD src_bpr,
              UWORD dst_x, UWORD dst_y, UWORD w, UWORD h)
{
    UWORD y;
    UBYTE *s, *d;
    for (y = 0; y < h; y++) {
        s = src + (src_y + y) * src_bpr + src_x;
        d = rtg.framebuffer + (dst_y + y) * rtg.bytes_per_row + dst_x;
        memcpy(d, s, w);
    }
}

void RTG_BlitMasked(UBYTE *src, UWORD src_x, UWORD src_y, UWORD src_bpr,
                    UWORD dst_x, UWORD dst_y, UWORD w, UWORD h,
                    UBYTE transparent)
{
    UWORD x, y;
    UBYTE *s, *d, c;
    for (y = 0; y < h && (dst_y + y) < rtg.height; y++) {
        s = src + (src_y + y) * src_bpr + src_x;
        d = rtg.framebuffer + (dst_y + y) * rtg.bytes_per_row + dst_x;
        for (x = 0; x < w && (dst_x + x) < rtg.width; x++) {
            c = s[x];
            if (c != transparent)
                d[x] = c;
        }
    }
}

void RTG_Refresh(void)
{
    struct RastPort *rp;
    if (!rtg.window) return;
    rp = rtg.window->RPort;
    if (!rp) return;
    WritePixelArray(rtg.framebuffer, 0, 0,
                    rtg.bytes_per_row, rp, 0, 0,
                    rtg.width, rtg.height, 8);
}

void RTG_WaitVBlank(void)
{
    if (timer_io) {
        timer_io->tr_node.io_Command = TRDERTASK;
        timer_io->tr_time.tv_secs = 0;
        timer_io->tr_time.tv_micro = 16667;
        DoIO((struct IORequest *)timer_io);
    }
}
