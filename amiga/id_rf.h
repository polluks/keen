#ifndef __ID_RF__
#define __ID_RF__

#include "id_heads.h"

#define MINTICS       2
#define MAXTICS       6
#define MAPBORDER     2
#define MAXSPRITES    100
#define MAXANIMTILES  90
#define MAXANIMTYPES  50
#define MAXMAPHEIGHT  128
#define PRIORITIES    4
#define MASKEDTILEPRIORITY 3
#define TILEGLOBAL    256
#define PIXGLOBAL     16
#define G_T_SHIFT     8
#define G_P_SHIFT     4
#define P_T_SHIFT     4
#define PORTTILESWIDE 21
#define PORTTILESHIGH 14
#define UPDATEWIDE    (PORTTILESWIDE+1)
#define UPDATEHIGH    PORTTILESHIGH

typedef enum { spritedraw, maskdraw } drawtype;

extern boolean  compatability;
extern unsigned tics;
extern long     lasttimecount;
extern unsigned originxglobal, originyglobal;
extern unsigned originxtile, originytile;
extern unsigned originxscreen, originyscreen;
extern unsigned mapwidth, mapheight, mapbyteswide, mapwordswide;
extern unsigned mapbytesextra, mapwordsextra;
extern unsigned mapbwidthtable[MAXMAPHEIGHT];
extern unsigned originxmin, originxmax, originymin, originymax;
extern unsigned masterofs;
extern byte     *updateptr;
extern unsigned blockstarts[UPDATEWIDE*UPDATEHIGH];
extern unsigned updatemapofs[UPDATEWIDE*UPDATEHIGH];
extern unsigned uwidthtable[UPDATEHIGH];

#define UPDATETERMINATE 0x0301

void RF_Startup(void);
void RF_Shutdown(void);
void RF_NewMap(void);
void RF_MarkTileGraphics(void);
void RF_NewPosition(unsigned x, unsigned y);
void RF_Scroll(int x, int y);
void RF_PlaceSprite(void **user, unsigned globalx, unsigned globaly,
    unsigned spritenumber, drawtype draw, int priority);
void RF_RemoveSprite(void **user);
void RF_Refresh(void);
void RF_ForceRefresh(void);
void RF_SetRefreshHook(void (*func)(void));

void RFL_NewRow(int dir);
void RFL_NewTile(unsigned updateoffset);
void RFL_MaskForegroundTiles(void);
void RFL_UpdateTiles(void);
void VWL_UpdateScreenBlocks(void);

#endif
