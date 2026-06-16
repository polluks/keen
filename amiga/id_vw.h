#ifndef __ID_VW__
#define __ID_VW__

#include "id_heads.h"

#define G_P_SHIFT    4
#define SCREENWIDTH  64
#define CHARWIDTH    1
#define TILEWIDTH    2

#define VIRTUALHEIGHT 300
#define VIRTUALWIDTH  512

#define MAXSHIFTS     8

#define WHITE     15
#define BLACK      0
#define FIRSTCOLOR 1
#define SECONDCOLOR 12
#define F_WHITE    0
#define F_BLACK   15
#define F_FIRSTCOLOR  14
#define F_SECONDCOLOR  3

typedef struct {
    int width, height, orgx, orgy;
    int xl, yl, xh, yh;
    int shifts;
} spritetabletype;

typedef struct {
    unsigned sourceoffset[MAXSHIFTS];
    unsigned planesize[MAXSHIFTS];
    unsigned width[MAXSHIFTS];
    byte data[];
} spritetype;

typedef struct {
    int width, height;
} pictabletype;

typedef struct {
    int height;
    int location[256];
    char width[256];
} fontstruct;

typedef enum { CGAgr, EGAgr, VGAgr } grtype;

extern grtype   grmode;
extern unsigned bufferofs;
extern unsigned displayofs;
extern unsigned panx, pany;
extern unsigned pansx, pansy;
extern unsigned panadjust;
extern unsigned linewidth;
extern unsigned ylookup[VIRTUALHEIGHT];
extern boolean  screenfaded;
extern pictabletype *pictable;
extern pictabletype *picmtable;
extern spritetabletype *spritetable;
extern int  px, py;
extern byte pdrawmode, fontcolor;
extern int  cursorvisible;
extern unsigned **shifttabletable;

void VW_Startup(void);
void VW_Shutdown(void);

void VW_SetLineWidth(int width);
void VW_SetScreen(unsigned CRTC, unsigned pelpan);
void VW_SetScreenMode(int grmode);
void VW_ClearVideo(int color);
void VW_WaitVBL(int number);
void VW_ColorBorder(int color);
void VW_SetDefaultColors(void);
void VW_FadeOut(void);
void VW_FadeIn(void);
void VW_FadeUp(void);
void VW_FadeDown(void);

void VW_MaskBlock(memptr segm, unsigned ofs, unsigned dest,
    unsigned wide, unsigned height, unsigned planesize);
void VW_MemToScreen(memptr source, unsigned dest,
    unsigned width, unsigned height);
void VW_ScreenToMem(unsigned source, memptr dest,
    unsigned width, unsigned height);
void VW_ScreenToScreen(unsigned source, unsigned dest,
    unsigned width, unsigned height);

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile);

#define VW_DrawTile8M(x,y,t) \
    VW_MaskBlock(grsegs[STARTTILE8M],(t)*40,bufferofs+ylookup[y]+(x),1,8,8)
#define VW_DrawTile16(x,y,t) \
    VW_MemToScreen(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),2,16)
#define VW_DrawTile16M(x,y,t) \
    VW_MaskBlock(grsegs[STARTTILE16M],(t)*160,bufferofs+ylookup[y]+(x),2,16,32)

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum);
void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum);

void VW_MeasurePropString(char *string, word *width, word *height);
void VW_MeasureMPropString(char *string, word *width, word *height);
void VW_DrawPropString(char *string);
void VW_DrawMPropString(char *string);
void VW_DrawSprite(int x, int y, unsigned sprite);
void VW_Plot(unsigned x, unsigned y, unsigned color);
void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color);
void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color);
void VW_Bar(unsigned x, unsigned y, unsigned width, unsigned height,
    unsigned color);

void VW_InitDoubleBuffer(void);
void VW_FixRefreshBuffer(void);
int  VW_MarkUpdateBlock(int x1, int y1, int x2, int y2);
void VW_UpdateScreen(void);
void VW_CGAFullUpdate(void);

void VW_ShowCursor(void);
void VW_HideCursor(void);
void VW_MoveCursor(int x, int y);
void VW_SetCursor(int spritenum);

void VWB_DrawTile8(int x, int y, int tile);
void VWB_DrawTile8M(int x, int y, int tile);
void VWB_DrawTile16(int x, int y, int tile);
void VWB_DrawTile16M(int x, int y, int tile);
void VWB_DrawPic(int x, int y, int chunknum);
void VWB_DrawMPic(int x, int y, int chunknum);
void VWB_Bar(int x, int y, int width, int height, int color);
void VWB_DrawPropString(char *string);
void VWB_DrawMPropString(char *string);
void VWB_DrawSprite(int x, int y, int chunknum);
void VWB_Plot(int x, int y, int color);
void VWB_Hlin(int x1, int x2, int y, int color);
void VWB_Vlin(int y1, int y2, int x, int color);

#endif
