#include "id_heads.h"
#include "rtg_driver.h"

#define VIEWWIDTH  40
#define PIXTOBLOCK 4
#define SCREENXMASK (~7)
#define SCREENXPLUS (7)
#define SCREENXDIV  8

grtype   grmode;
unsigned bufferofs;
unsigned displayofs;
unsigned panx, pany;
unsigned pansx, pansy;
unsigned panadjust;
unsigned linewidth;
unsigned ylookup[VIRTUALHEIGHT];
boolean  screenfaded;

pictabletype *pictable;
pictabletype *picmtable;
spritetabletype *spritetable;
int  px, py;
byte pdrawmode, fontcolor;
unsigned **shifttabletable;

static int bordercolor;
int cursorvisible;
static int cursornumber, cursorwidth, cursorheight, cursorx, cursory;
static memptr cursorsave;
static unsigned cursorspot;
static unsigned bufferwidth, bufferheight;

static unsigned char leftmask[8] = {0xff,0x7f,0x3f,0x1f,0xf,7,3,1};
static unsigned char rightmask[8] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

void VW_Startup(void)
{
    grmode = EGAgr;
    screenfaded = false;
    cursorvisible = 0;

    VW_SetLineWidth(SCREENWIDTH);

    pictable = NULL;
    picmtable = NULL;
    spritetable = NULL;
}

void VW_Shutdown(void)
{
    RTG_CloseScreen();
}

void VW_SetScreenMode(int mode)
{
    if (mode == TEXTGR) {
        RTG_CloseScreen();
    } else if (mode == EGAGR) {
        if (!rtg.framebuffer)
            RTG_OpenScreen();
        VW_SetLineWidth(SCREENWIDTH);
    }
}

void VW_ColorBorder(int color)
{
    bordercolor = color;
}

void VW_SetDefaultColors(void)
{
    screenfaded = false;
}

void VW_FadeOut(void)
{
    screenfaded = true;
}

void VW_FadeIn(void)
{
    screenfaded = false;
}

void VW_FadeUp(void)
{
    screenfaded = true;
}

void VW_FadeDown(void)
{
    screenfaded = false;
}

void VW_WaitVBL(int number)
{
    int i;
    for (i = 0; i < number; i++)
        RTG_WaitVBlank();
}

void VW_SetLineWidth(int width)
{
    int i, offset;
    linewidth = width;
    offset = 0;
    for (i = 0; i < VIRTUALHEIGHT; i++) {
        ylookup[i] = offset;
        offset += width;
    }
}

void VW_ClearVideo(int color)
{
    unsigned dest;
    unsigned i, j;
    for (i = 0; i < VIRTUALHEIGHT; i++) {
        dest = bufferofs + ylookup[i];
        for (j = 0; j < SCREENWIDTH; j++)
            rtg.framebuffer[dest + j] = (byte)color;
    }
}

void VW_SetScreen(unsigned CRTC, unsigned pelpan)
{
    displayofs = CRTC;
}

void VW_MaskBlock(memptr segm, unsigned ofs, unsigned dest,
    unsigned wide, unsigned height, unsigned planesize)
{
    byte *s = (byte *)segm + ofs;
    int x, y, p, bit;
    byte c, mask;
    UWORD dst_x = (dest - bufferofs) % SCREENWIDTH;
    UWORD dst_y = ((dest - bufferofs) / SCREENWIDTH) * 8;

    for (y = 0; y < (int)height; y++) {
        for (x = 0; x < (int)wide; x++) {
            c = 0;
            mask = 0x80;
            for (bit = 0; bit < 8; bit++) {
                for (p = 0; p < 4; p++) {
                    if (s[p * planesize + y * wide + x] & mask)
                        c |= (1 << p);
                }
                mask >>= 1;
            }
            if (c)
                rtg.framebuffer[(dst_y + y) * rtg.bytes_per_row + dst_x * 8 + x] = c;
        }
    }
}

void VW_MemToScreen(memptr source, unsigned dest,
    unsigned width, unsigned height)
{
    byte *s = (byte *)source;
    int x, y, p, bit;
    byte c, mask;
    UWORD dst_x = (dest - bufferofs) % SCREENWIDTH;
    UWORD dst_y = ((dest - bufferofs) / SCREENWIDTH);

    for (y = 0; y < (int)height; y++) {
        for (x = 0; x < (int)width; x++) {
            c = 0;
            mask = 0x80;
            for (bit = 0; bit < 8; bit++) {
                for (p = 0; p < 4; p++) {
                    if (s[p * width * height + y * width + x] & mask)
                        c |= (1 << p);
                }
                mask >>= 1;
            }
            if (c)
                rtg.framebuffer[(dst_y + y) * rtg.bytes_per_row + dst_x * 8 + x] = c;
        }
    }
}

void VW_ScreenToMem(unsigned source, memptr dest,
    unsigned width, unsigned height)
{
    byte *d = (byte *)dest;
    int x, y, p, bit;
    byte c, mask;
    UWORD src_x = (source - bufferofs) % SCREENWIDTH;
    UWORD src_y = ((source - bufferofs) / SCREENWIDTH);

    memset(d, 0, width * height * 4);

    for (y = 0; y < (int)height; y++) {
        for (x = 0; x < (int)width * 8; x++) {
            c = rtg.framebuffer[(src_y + y) * rtg.bytes_per_row + src_x * 8 + x];
            if (c) {
                mask = 0x80 >> (x & 7);
                for (p = 0; p < 4; p++) {
                    if (c & (1 << p))
                        d[p * width * height + y * width + (x >> 3)] |= mask;
                }
            }
        }
    }
}

void VW_ScreenToScreen(unsigned source, unsigned dest,
    unsigned width, unsigned height)
{
    memcpy(rtg.framebuffer + dest, rtg.framebuffer + source, width * height);
}

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum)
{
    int picnum = chunknum - STARTPICS;
    memptr source;
    unsigned dest, width, height;

    source = grsegs[chunknum];
    dest = ylookup[y] + x + bufferofs;
    width = pictable[picnum].width;
    height = pictable[picnum].height;

    VW_MemToScreen(source, dest, width, height);
}

void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum)
{
    int picnum = chunknum - STARTPICM;
    memptr source;
    unsigned dest, width, height;

    source = grsegs[chunknum];
    dest = ylookup[y] + x + bufferofs;
    width = picmtable[picnum].width;
    height = picmtable[picnum].height;

    VW_MaskBlock(source, 0, dest, width, height, width * height);
}

void VW_DrawSprite(int x, int y, unsigned chunknum)
{
    spritetabletype *spr;
    spritetype *block;
    unsigned dest, shift;

    spr = &spritetable[chunknum - STARTSPRITES];
    block = (spritetype *)grsegs[chunknum];

    y += spr->orgy >> G_P_SHIFT;
    x += spr->orgx >> G_P_SHIFT;

    shift = (x & 7) / 2;

    dest = bufferofs + ylookup[y];
    if (x >= 0)
        dest += x / SCREENXDIV;
    else
        dest += (x + 1) / SCREENXDIV;

    VW_MaskBlock(block, block->sourceoffset[shift], dest,
        block->width[shift], spr->height, block->planesize[shift]);
}

void VW_Plot(unsigned x, unsigned y, unsigned color)
{
    unsigned dest = bufferofs + ylookup[y] + x / SCREENXDIV;
    UWORD dst_x = (dest - bufferofs) % SCREENWIDTH;
    UWORD dst_y = ((dest - bufferofs) / SCREENWIDTH);

    if (dst_y < VIRTUALHEIGHT && dst_x * 8 + (x & 7) < rtg.width)
        rtg.framebuffer[dst_y * rtg.bytes_per_row + dst_x * 8 + (x & 7)] = (byte)color;
}

void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color)
{
    unsigned dest, xlb, xhb, mid;
    unsigned maskleft, maskright;
    UWORD dst_x, dst_y;

    xlb = xl / 8;
    xhb = xh / 8;

    maskleft  = leftmask[xl & 7];
    maskright = rightmask[xh & 7];

    mid = xhb - xlb - 1;
    dest = bufferofs + ylookup[y] + xlb;
    dst_x = (dest - bufferofs) % SCREENWIDTH;
    dst_y = ((dest - bufferofs) / SCREENWIDTH);

    if (xlb == xhb) {
        maskleft &= maskright;
        dst_x = dst_x * 8;
        while (maskleft) {
            if (maskleft & 0x80)
                rtg.framebuffer[dst_y * rtg.bytes_per_row + dst_x] = (byte)color;
            maskleft <<= 1;
            dst_x++;
        }
        return;
    }

    dst_x = dst_x * 8;
    {
        unsigned m = maskleft;
        unsigned dx = dst_x;
        while (m) {
            if (m & 0x80)
                rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
            m <<= 1;
            dx++;
        }
    }

    {
        unsigned dx;
        for (dx = dst_x + 8; dx < dst_x + 8 + mid * 8; dx++)
            rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
    }

    {
        unsigned m = maskright;
        unsigned dx = dst_x + (mid + 1) * 8;
        while (m) {
            if (m & 0x80)
                rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
            m <<= 1;
            dx++;
        }
    }
}

void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color)
{
    unsigned dest = bufferofs + ylookup[yl] + x / SCREENXDIV;
    UWORD dst_x = (dest - bufferofs) % SCREENWIDTH;
    UWORD dst_y = ((dest - bufferofs) / SCREENWIDTH);
    UWORD i;

    for (i = 0; i <= (yh - yl); i++) {
        if ((dst_y + i) < VIRTUALHEIGHT)
            rtg.framebuffer[(dst_y + i) * rtg.bytes_per_row + dst_x * 8] = (byte)color;
    }
}

void VW_Bar(unsigned x, unsigned y, unsigned width, unsigned height,
    unsigned color)
{
    unsigned dest, xh, xlb, xhb, mid;
    unsigned maskleft, maskright;
    UWORD dst_x, dst_y;
    UWORD i;

    xh = x + width - 1;
    xlb = x / 8;
    xhb = xh / 8;

    maskleft  = leftmask[x & 7];
    maskright = rightmask[xh & 7];

    mid = xhb - xlb - 1;

    for (i = 0; i < height; i++) {
        dest = bufferofs + ylookup[y + i] + xlb;
        dst_x = (dest - bufferofs) % SCREENWIDTH;
        dst_y = ((dest - bufferofs) / SCREENWIDTH);

        if (xlb == xhb) {
            unsigned m = maskleft & maskright;
            unsigned dx = dst_x * 8;
            while (m) {
                if (m & 0x80)
                    rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
                m <<= 1;
                dx++;
            }
            continue;
        }

        {
            unsigned m = maskleft;
            unsigned dx = dst_x * 8;
            while (m) {
                if (m & 0x80)
                    rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
                m <<= 1;
                dx++;
            }
        }

        {
            unsigned dx;
            for (dx = dst_x * 8 + 8; dx < dst_x * 8 + 8 + mid * 8; dx++)
                rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
        }

        {
            unsigned m = maskright;
            unsigned dx = dst_x * 8 + (mid + 1) * 8;
            while (m) {
                if (m & 0x80)
                    rtg.framebuffer[dst_y * rtg.bytes_per_row + dx] = (byte)color;
                m <<= 1;
                dx++;
            }
        }
    }
}

static void VWL_MeasureString(char *string, word *width, word *height,
    fontstruct *font)
{
    *height = font->height;
    for (*width = 0; *string; string++)
        *width += font->width[(int)*string];
}

void VW_MeasurePropString(char *string, word *width, word *height)
{
    VWL_MeasureString(string, width, height, (fontstruct *)grsegs[STARTFONT]);
}

void VW_MeasureMPropString(char *string, word *width, word *height)
{
    VWL_MeasureString(string, width, height, (fontstruct *)grsegs[STARTFONTM]);
}

void VW_DrawPropString(char *string)
{
    fontstruct *font;
    int x, y, w, h;
    byte *src, *s;
    int i, j, k;
    unsigned dest;

    font = (fontstruct *)grsegs[STARTFONT];
    dest = bufferofs + ylookup[py] + px / SCREENXDIV;

    x = px;
    y = py;

    while (*string) {
        w = font->width[(int)*string];
        h = font->height;
        s = (byte *)font + font->location[(int)*string];

        for (j = 0; j < h; j++) {
            src = s + j * w;
            for (i = 0; i < w; i++) {
                if (src[i]) {
                    UWORD dx = x + i;
                    UWORD dy = y + j;
                    rtg.framebuffer[dy * rtg.bytes_per_row + dx] = src[i];
                }
            }
        }
        x += w;
        string++;
    }
    px = x;
}

void VW_DrawMPropString(char *string)
{
    fontstruct *font;
    int x, y, w, h, i, j;
    byte *src, *s;
    unsigned dest;

    font = (fontstruct *)grsegs[STARTFONTM];
    dest = bufferofs + ylookup[py] + px / SCREENXDIV;

    x = px;
    y = py;

    while (*string) {
        w = font->width[(int)*string];
        h = font->height;
        s = (byte *)font + font->location[(int)*string];

        for (j = 0; j < h; j++) {
            src = s + j * w;
            for (i = 0; i < w; i++) {
                if (src[i]) {
                    UWORD dx = x + i;
                    UWORD dy = y + j;
                    rtg.framebuffer[dy * rtg.bytes_per_row + dx] = src[i];
                }
            }
        }
        x += w;
        string++;
    }
    px = x;
}

void VWL_DrawCursor(void)
{
    cursorspot = bufferofs + ylookup[cursory + pansy]
        + (cursorx + pansx) / SCREENXDIV;
    VW_ScreenToMem(cursorspot, cursorsave, cursorwidth, cursorheight);
    VWB_DrawSprite(cursorx, cursory, cursornumber);
}

void VWL_EraseCursor(void)
{
    VW_MemToScreen(cursorsave, cursorspot, cursorwidth, cursorheight);
    VW_MarkUpdateBlock((cursorx + pansx) & SCREENXMASK, cursory + pansy,
        ((cursorx + pansx) & SCREENXMASK) + cursorwidth * SCREENXDIV - 1,
        cursory + pansy + cursorheight - 1);
}

void VW_ShowCursor(void) { cursorvisible++; }
void VW_HideCursor(void) { cursorvisible--; }
void VW_MoveCursor(int x, int y) { cursorx = x; cursory = y; }

void VW_SetCursor(int spritenum)
{
    if (cursornumber) {
        MM_SetLock(&grsegs[cursornumber], false);
        MM_FreePtr(&cursorsave);
    }
    cursornumber = spritenum;
    CA_CacheGrChunk(spritenum);
    MM_SetLock(&grsegs[cursornumber], true);
    cursorwidth = spritetable[spritenum - STARTSPRITES].width + 1;
    cursorheight = spritetable[spritenum - STARTSPRITES].height;
    MM_GetPtr(&cursorsave, cursorwidth * cursorheight * 5);
}

void VW_InitDoubleBuffer(void)
{
    VW_SetScreen(displayofs + panadjust, 0);
}

void VW_FixRefreshBuffer(void)
{
    VW_ScreenToScreen(displayofs, bufferofs,
        PORTTILESWIDE * 4 * CHARWIDTH, PORTTILESHIGH * 16);
}

void VW_QuitDoubleBuffer(void) {}

int VW_MarkUpdateBlock(int x1, int y1, int x2, int y2)
{
    int x, y, xt1, yt1, xt2, yt2, nextline;
    byte *mark;

    xt1 = x1 >> PIXTOBLOCK;
    yt1 = y1 >> PIXTOBLOCK;
    xt2 = x2 >> PIXTOBLOCK;
    yt2 = y2 >> PIXTOBLOCK;

    if (xt1 < 0) xt1 = 0;
    else if (xt1 >= UPDATEWIDE - 1) return 0;
    if (yt1 < 0) yt1 = 0;
    else if (yt1 > UPDATEHIGH) return 0;
    if (xt2 < 0) return 0;
    else if (xt2 >= UPDATEWIDE - 1) xt2 = UPDATEWIDE - 2;
    if (yt2 < 0) return 0;
    else if (yt2 >= UPDATEHIGH) yt2 = UPDATEHIGH - 1;

    mark = updateptr + uwidthtable[yt1] + xt1;
    nextline = UPDATEWIDE - (xt2 - xt1) - 1;

    for (y = yt1; y <= yt2; y++) {
        for (x = xt1; x <= xt2; x++)
            *mark++ = 1;
        mark += nextline;
    }
    return 1;
}

void VW_UpdateScreen(void)
{
    if (cursorvisible > 0)
        VWL_DrawCursor();

    VWL_UpdateScreenBlocks();

    if (cursorvisible > 0)
        VWL_EraseCursor();

    RTG_Refresh();
}

void VWB_DrawTile8(int x, int y, int tile)
{
    x += pansx;
    y += pansy;
    if (VW_MarkUpdateBlock(x & SCREENXMASK, y, (x & SCREENXMASK) + 7, y + 7))
        VW_DrawTile8(x / SCREENXDIV, y, tile);
}

void VWB_DrawTile8M(int x, int y, int tile)
{
    int xb;
    x += pansx;
    y += pansy;
    xb = x / SCREENXDIV;
    if (VW_MarkUpdateBlock(x & SCREENXMASK, y, (x & SCREENXMASK) + 7, y + 7))
        VW_DrawTile8M(xb, y, tile);
}

void VWB_DrawTile16(int x, int y, int tile)
{
    x += pansx;
    y += pansy;
    if (VW_MarkUpdateBlock(x & SCREENXMASK, y, (x & SCREENXMASK) + 15, y + 15))
        VW_DrawTile16(x / SCREENXDIV, y, tile);
}

void VWB_DrawTile16M(int x, int y, int tile)
{
    int xb;
    x += pansx;
    y += pansy;
    xb = x / SCREENXDIV;
    if (VW_MarkUpdateBlock(x & SCREENXMASK, y, (x & SCREENXMASK) + 15, y + 15))
        VW_DrawTile16M(xb, y, tile);
}

void VWB_DrawPic(int x, int y, int chunknum)
{
    int picnum = chunknum - STARTPICS;
    memptr source;
    unsigned dest, width, height;

    x += pansx;
    y += pansy;
    x /= SCREENXDIV;

    source = grsegs[chunknum];
    dest = ylookup[y] + x + bufferofs;
    width = pictable[picnum].width;
    height = pictable[picnum].height;

    if (VW_MarkUpdateBlock(x * SCREENXDIV, y,
        (x + width) * SCREENXDIV - 1, y + height - 1))
        VW_MemToScreen(source, dest, width, height);
}

void VWB_DrawMPic(int x, int y, int chunknum)
{
    int picnum = chunknum - STARTPICM;
    memptr source;
    unsigned dest, width, height;

    x += pansx;
    y += pansy;
    x /= SCREENXDIV;

    source = grsegs[chunknum];
    dest = ylookup[y] + x + bufferofs;
    width = picmtable[picnum].width;
    height = picmtable[picnum].height;

    if (VW_MarkUpdateBlock(x * SCREENXDIV, y,
        (x + width) * SCREENXDIV - 1, y + height - 1))
        VW_MaskBlock(source, 0, dest, width, height, width * height);
}

void VWB_Bar(int x, int y, int width, int height, int color)
{
    x += pansx;
    y += pansy;
    if (VW_MarkUpdateBlock(x, y, x + width, y + height - 1))
        VW_Bar(x, y, width, height, color);
}

void VWB_DrawPropString(char *string)
{
    int x, y;
    x = px + pansx;
    y = py + pansy;
    VW_DrawPropString(string);
    VW_MarkUpdateBlock(x, y, x + bufferwidth * 8 - 1, y + bufferheight - 1);
}

void VWB_DrawMPropString(char *string)
{
    int x, y;
    x = px + pansx;
    y = py + pansy;
    VW_DrawMPropString(string);
    VW_MarkUpdateBlock(x, y, x + bufferwidth * 8 - 1, y + bufferheight - 1);
}

void VWB_DrawSprite(int x, int y, int chunknum)
{
    spritetabletype *spr;
    spritetype *block;
    unsigned dest, shift, width, height;

    x += pansx;
    y += pansy;

    spr = &spritetable[chunknum - STARTSPRITES];
    block = (spritetype *)grsegs[chunknum];

    y += spr->orgy >> G_P_SHIFT;
    x += spr->orgx >> G_P_SHIFT;

    shift = (x & 7) / 2;

    dest = bufferofs + ylookup[y];
    if (x >= 0)
        dest += x / SCREENXDIV;
    else
        dest += (x + 1) / SCREENXDIV;

    width = block->width[shift];
    height = spr->height;

    if (VW_MarkUpdateBlock(x & SCREENXMASK, y,
        (x & SCREENXMASK) + width * SCREENXDIV - 1, y + height - 1))
        VW_MaskBlock(block, block->sourceoffset[shift], dest,
            width, height, block->planesize[shift]);
}

void VWB_Plot(int x, int y, int color)
{
    x += pansx;
    y += pansy;
    if (VW_MarkUpdateBlock(x, y, x, y))
        VW_Plot(x, y, color);
}

void VWB_Hlin(int x1, int x2, int y, int color)
{
    x1 += pansx;
    x2 += pansx;
    y += pansy;
    if (VW_MarkUpdateBlock(x1, y, x2, y))
        VW_Hlin(x1, x2, y, color);
}

void VWB_Vlin(int y1, int y2, int x, int color)
{
    x += pansx;
    y1 += pansy;
    y2 += pansy;
    if (VW_MarkUpdateBlock(x, y1, x, y2))
        VW_Vlin(y1, y2, x, color);
}
