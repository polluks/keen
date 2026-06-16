#include "id_heads.h"
#include "rtg_driver.h"

#define SCREENTILESWIDE 20
#define SCREENTILESHIGH 13
#define SCREENSPACE     (SCREENWIDTH*240)
#define FREEEGAMEM      (0x10000l-3l*SCREENSPACE)

#define UPDATESCREENSIZE (UPDATEWIDE*PORTTILESHIGH+2)
#define UPDATESPARESIZE  (UPDATEWIDE*2+4)
#define UPDATESIZE       (UPDATESCREENSIZE+2*UPDATESPARESIZE)

#define G_EGASX_SHIFT 7
#define G_CGASX_SHIFT 6
#define G_SY_SHIFT    4

unsigned SX_T_SHIFT;
#define SY_T_SHIFT 4

#define EGAPORTSCREENWIDE 42
#define CGAPORTSCREENWIDE 84
#define PORTSCREENHIGH    224

typedef struct spriteliststruct {
    int  screenx, screeny;
    int  width, height;
    unsigned grseg, sourceofs, planesize;
    drawtype draw;
    unsigned tilex, tiley, tilewide, tilehigh;
    int  priority, updatecount;
    struct spriteliststruct **prevptr, *nextsprite;
} spritelisttype;

typedef struct {
    int  screenx, screeny;
    int  width, height;
} eraseblocktype;

typedef struct {
    unsigned current;
    int  count;
} tiletype;

typedef struct animtilestruct {
    unsigned x, y, tile;
    tiletype *chain;
    unsigned *mapplane;
    struct animtilestruct **prevptr, *nexttile;
} animtiletype;

unsigned tics;
long    lasttimecount;
boolean compatability;

unsigned mapwidth, mapheight, mapbyteswide, mapwordswide;
unsigned mapbytesextra, mapwordsextra;
unsigned mapbwidthtable[MAXMAPHEIGHT];

unsigned originxglobal, originyglobal;
unsigned originxtile, originytile;
unsigned originxscreen, originyscreen;
unsigned originmap;
unsigned originxmin, originxmax, originymin, originymax;
unsigned masterofs;

unsigned blockstarts[UPDATEWIDE*UPDATEHIGH];
unsigned updatemapofs[UPDATEWIDE*UPDATEHIGH];
unsigned uwidthtable[PORTTILESHIGH];

byte update[2][UPDATESIZE];
byte *updateptr, *baseupdateptr;
byte *updatestart[2], *baseupdatestart[2];

static char scratch[20], str[20];
tiletype allanims[MAXANIMTYPES];
unsigned numanimchains;
void (*refreshvector)(void);

unsigned screenstart[3] = {0, SCREENSPACE, SCREENSPACE*2};
unsigned xpanmask;
unsigned screenpage;
unsigned otherpage;

unsigned tilecache[NUMTILE16];

spritelisttype spritearray[MAXSPRITES], *prioritystart[PRIORITIES],
               *spritefreeptr;
animtiletype animarray[MAXANIMTILES], *animhead, *animfreeptr;
int animfreespot;
eraseblocktype eraselist[2][MAXSPRITES], *eraselistptr[2];



void RF_Startup(void)
{
    int i, x, y;
    unsigned *blockstart;

    for (i = 0; i < PORTTILESHIGH; i++)
        uwidthtable[i] = UPDATEWIDE * i;

    originxmin = originymin = MAPBORDER * TILEGLOBAL;

    eraselistptr[0] = &eraselist[0][0];
    eraselistptr[1] = &eraselist[1][0];

    SX_T_SHIFT = 1;

    baseupdatestart[0] = &update[0][UPDATESPARESIZE];
    baseupdatestart[1] = &update[1][UPDATESPARESIZE];

    screenpage = 0;
    otherpage = 1;
    displayofs = screenstart[screenpage];
    bufferofs = screenstart[otherpage];
    masterofs = screenstart[2];

    updateptr = baseupdatestart[otherpage];

    blockstart = &blockstarts[0];
    for (y = 0; y < UPDATEHIGH; y++)
        for (x = 0; x < UPDATEWIDE; x++)
            *blockstart++ = SCREENWIDTH * 16 * y + x * TILEWIDTH;

    xpanmask = 6;
}

void RF_Shutdown(void) {}

void RF_NewMap(void)
{
    int i, x, y;
    unsigned spot, *table;

    mapwidth = mapheaderseg[mapon]->width;
    mapbyteswide = 2 * mapwidth;
    mapheight = mapheaderseg[mapon]->height;
    mapwordsextra = mapwidth - PORTTILESWIDE;
    mapbytesextra = 2 * mapwordsextra;

    spot = 0;
    for (i = 0; i < (int)mapheight; i++) {
        mapbwidthtable[i] = spot;
        spot += mapbyteswide;
    }

    table = &updatemapofs[0];
    for (y = 0; y < PORTTILESHIGH; y++)
        for (x = 0; x < UPDATEWIDE; x++)
            *table++ = mapbwidthtable[y] + x * 2;

    originxmax = (mapwidth - MAPBORDER - SCREENTILESWIDE) * TILEGLOBAL;
    originymax = (mapheight - MAPBORDER - SCREENTILESHIGH) * TILEGLOBAL;
    if (originxmax < originxmin) originxmax = originxmin;
    if (originymax < originymin) originymax = originymin;

    RFL_InitSpriteList();
    RFL_InitAnimList();

    lasttimecount = TimeCount;
    tics = 1;
}

void RF_MarkTileGraphics(void)
{
    unsigned size;
    int  tile, next, anims;
    unsigned *start, *end, *info;
    unsigned i, tilehigh;

    memset(allanims, 0, sizeof(allanims));
    numanimchains = 0;

    size = mapwidth * mapheight;

    start = mapsegs[0];
    info = mapsegs[2];
    end = start + size;
    do {
        tile = *start++;
        if (tile >= 0) {
            CA_MarkGrChunk(STARTTILE16 + tile);
            if (tinf[ANIM + tile]) {
                for (i = 0; i < numanimchains; i++)
                    if (allanims[i].current == (unsigned)tile) {
                        *info = (unsigned)&allanims[i];
                        goto nextback;
                    }
                if (i >= MAXANIMTYPES)
                    Quit("RF_MarkTileGraphics: Too many unique animated tiles!");
                allanims[i].current = tile;
                allanims[i].count = tinf[SPEED + tile];
                *info = (unsigned)&allanims[i];
                numanimchains++;
                anims = 0;
                next = tile + (signed char)(tinf[ANIM + tile]);
                while (next != tile) {
                    CA_MarkGrChunk(STARTTILE16 + next);
                    next += (signed char)(tinf[ANIM + next]);
                    if (++anims > 20)
                        Quit("MarkTileGraphics: Unending animation!");
                }
            }
        }
nextback:
        info++;
    } while (start < end);

    start = mapsegs[1];
    info = mapsegs[2];
    end = start + size;
    do {
        tile = *start++;
        if (tile >= 0) {
            CA_MarkGrChunk(STARTTILE16M + tile);
            if (tinf[MANIM + tile]) {
                tilehigh = tile | 0x8000;
                for (i = 0; i < numanimchains; i++)
                    if (allanims[i].current == tilehigh) {
                        *info = (unsigned)&allanims[i];
                        goto nextfront;
                    }
                if (i >= MAXANIMTYPES)
                    Quit("RF_MarkTileGraphics: Too many unique animated tiles!");
                allanims[i].current = tilehigh;
                allanims[i].count = tinf[MSPEED + tile];
                *info = (unsigned)&allanims[i];
                numanimchains++;
                anims = 0;
                next = tile + (signed char)(tinf[MANIM + tile]);
                while (next != tile) {
                    CA_MarkGrChunk(STARTTILE16M + next);
                    next += (signed char)(tinf[MANIM + next]);
                    if (++anims > 20)
                        Quit("MarkTileGraphics: Unending animation!");
                }
            }
        }
nextfront:
        info++;
    } while (start < end);
}

void RFL_InitAnimList(void)
{
    int i;
    animfreeptr = &animarray[0];
    for (i = 0; i < MAXANIMTILES - 1; i++)
        animarray[i].nexttile = &animarray[i + 1];
    animarray[i].nexttile = NULL;
    animhead = NULL;
}

void RFL_CheckForAnimTile(unsigned x, unsigned y)
{
    unsigned tile, offset;
    unsigned *map;
    animtiletype *anim, *next;

    offset = mapbwidthtable[y] / 2 + x;

    map = mapsegs[0] + offset;
    tile = *map;
    if (tinf[ANIM + tile]) {
        if (!animfreeptr)
            Quit("RF_CheckForAnimTile: No free spots in tilearray!");
        anim = animfreeptr;
        animfreeptr = animfreeptr->nexttile;
        next = animhead;
        animhead = anim;
        if (next)
            next->prevptr = &anim->nexttile;
        anim->nexttile = next;
        anim->prevptr = &animhead;
        anim->x = x;
        anim->y = y;
        anim->tile = tile;
        anim->mapplane = map;
        anim->chain = (tiletype *)*(mapsegs[2] + offset);
    }

    map = mapsegs[1] + offset;
    tile = *map;
    if (tinf[MANIM + tile]) {
        if (!animfreeptr)
            Quit("RF_CheckForAnimTile: No free spots in tilearray!");
        anim = animfreeptr;
        animfreeptr = animfreeptr->nexttile;
        next = animhead;
        animhead = anim;
        if (next)
            next->prevptr = &anim->nexttile;
        anim->nexttile = next;
        anim->prevptr = &animhead;
        anim->x = x;
        anim->y = y;
        anim->tile = tile;
        anim->mapplane = map;
        anim->chain = (tiletype *)*(mapsegs[2] + offset);
    }
}

void RFL_RemoveAnimsOnX(unsigned x)
{
    animtiletype *current, *next;
    current = animhead;
    while (current) {
        if (current->x == x) {
            *(void **)current->prevptr = current->nexttile;
            if (current->nexttile)
                current->nexttile->prevptr = current->prevptr;
            next = current->nexttile;
            current->nexttile = animfreeptr;
            animfreeptr = current;
            current = next;
        } else {
            current = current->nexttile;
        }
    }
}

void RFL_RemoveAnimsOnY(unsigned y)
{
    animtiletype *current, *next;
    current = animhead;
    while (current) {
        if (current->y == y) {
            *(void **)current->prevptr = current->nexttile;
            if (current->nexttile)
                current->nexttile->prevptr = current->prevptr;
            next = current->nexttile;
            current->nexttile = animfreeptr;
            animfreeptr = current;
            current = next;
        } else {
            current = current->nexttile;
        }
    }
}

void RFL_AnimateTiles(void)
{
    animtiletype *current;
    unsigned updateofs, tile, x, y;
    tiletype *anim;

    anim = &allanims[0];
    while (anim->current) {
        anim->count -= tics;
        while (anim->count < 1) {
            if (anim->current & 0x8000) {
                tile = anim->current & 0x7fff;
                tile += (signed char)tinf[MANIM + tile];
                anim->count += tinf[MSPEED + tile];
                tile |= 0x8000;
            } else {
                tile = anim->current;
                tile += (signed char)tinf[ANIM + tile];
                anim->count += tinf[SPEED + tile];
            }
            anim->current = tile;
        }
        anim++;
    }

    current = animhead;
    while (current) {
        tile = current->chain->current;
        if (tile != current->tile) {
            current->tile = tile;
            *(current->mapplane) = tile & 0x7fff;
            if (tile < 0x8000)
                tilecache[tile] = 0;

            x = current->x - originxtile;
            y = current->y - originytile;

            if (x >= PORTTILESWIDE || y >= PORTTILESHIGH)
                Quit("RFL_AnimateTiles: Out of bounds!");

            updateofs = uwidthtable[y] + x;
            RFL_NewTile(updateofs);
        }
        current = current->nexttile;
    }
}

void RFL_InitSpriteList(void)
{
    int i;
    spritefreeptr = &spritearray[0];
    for (i = 0; i < MAXSPRITES - 1; i++)
        spritearray[i].nextsprite = &spritearray[i + 1];
    spritearray[i].nextsprite = NULL;
    memset(prioritystart, 0, sizeof(prioritystart));
}

void RFL_CalcOriginStuff(long x, long y)
{
    if (x < originxmin) x = originxmin;
    else if (x > originxmax) x = originxmax;
    if (y < originymin) y = originymin;
    else if (y > originymax) y = originymax;

    originxglobal = x;
    originyglobal = y;
    originxtile = originxglobal >> G_T_SHIFT;
    originytile = originyglobal >> G_T_SHIFT;
    originxscreen = originxtile << SX_T_SHIFT;
    originyscreen = originytile << SY_T_SHIFT;
    originmap = mapbwidthtable[originytile] + originxtile * 2;

    panx = (originxglobal >> G_P_SHIFT) & 15;
    pansx = panx & 8;
    pany = pansy = (originyglobal >> G_P_SHIFT) & 15;
    panadjust = panx / 8 + ylookup[pany];
}

void RF_SetRefreshHook(void (*func)(void))
{
    refreshvector = func;
}

void RFL_NewRow(int dir)
{
    unsigned count, updatespot, updatestep;
    int x, y, xstep, ystep;

    switch (dir) {
    case 0:
        updatespot = 0;
        updatestep = 1;
        x = originxtile;
        y = originytile;
        xstep = 1;
        ystep = 0;
        count = PORTTILESWIDE;
        break;
    case 1:
        updatespot = PORTTILESWIDE - 1;
        updatestep = UPDATEWIDE;
        x = originxtile + PORTTILESWIDE - 1;
        y = originytile;
        xstep = 0;
        ystep = 1;
        count = PORTTILESHIGH;
        break;
    case 2:
        updatespot = UPDATEWIDE * (PORTTILESHIGH - 1);
        updatestep = 1;
        x = originxtile;
        y = originytile + PORTTILESHIGH - 1;
        xstep = 1;
        ystep = 0;
        count = PORTTILESWIDE;
        break;
    case 3:
        updatespot = 0;
        updatestep = UPDATEWIDE;
        x = originxtile;
        y = originytile;
        xstep = 0;
        ystep = 1;
        count = PORTTILESHIGH;
        break;
    default:
        Quit("RFL_NewRow: Bad dir!");
        return;
    }

    while (count--) {
        RFL_NewTile(updatespot);
        RFL_CheckForAnimTile(x, y);
        updatespot += updatestep;
        x += xstep;
        y += ystep;
    }
}

void RF_ForceRefresh(void)
{
    RF_NewPosition(originxglobal, originyglobal);
    RF_Refresh();
    RF_Refresh();
}

static void RFL_OldRow(unsigned updatespot, unsigned count, unsigned step)
{
    unsigned *s;
    unsigned tile;

    s = (unsigned *)(((byte *)mapsegs[0]) + updatespot);
    while (count--) {
        tile = *s;
        if (tile < NUMTILE16)
            tilecache[tile] = 0;
        s = (unsigned *)(((byte *)s) + step);
    }
}

void RF_NewPosition(unsigned x, unsigned y)
{
    int mx, my;
    byte *page0ptr, *page1ptr;
    unsigned updatenum;

    RFL_CalcOriginStuff(x, y);
    RFL_InitAnimList();

    memset(tilecache, 0, sizeof(tilecache));

    updatestart[0] = baseupdatestart[0];
    updatestart[1] = baseupdatestart[1];

    page0ptr = updatestart[0] + PORTTILESWIDE;
    page1ptr = updatestart[1] + PORTTILESWIDE;

    updatenum = 0;
    for (my = 0; my < PORTTILESHIGH; my++) {
        for (mx = 0; mx < PORTTILESWIDE; mx++) {
            RFL_NewTile(updatenum);
            RFL_CheckForAnimTile(mx + originxtile, my + originytile);
            updatenum++;
        }
        updatenum++;
        *page0ptr = *page1ptr = 0;
        page0ptr += (PORTTILESWIDE + 1);
        page1ptr += (PORTTILESWIDE + 1);
    }
    *(word *)(page0ptr - PORTTILESWIDE) =
    *(word *)(page1ptr - PORTTILESWIDE) = UPDATETERMINATE;
}

void RF_Scroll(int x, int y)
{
    long neworgx, neworgy;
    int i, absdx, absdy;
    int oldxt, oldyt, move, yy;
    int deltax, deltay;
    unsigned updatespot;
    byte *update0, *update1;
    unsigned oldpanx, oldpanadjust, oldoriginmap;
    unsigned oldscreen, newscreen, screencopy;
    int screenmove;

    oldxt = originxtile;
    oldyt = originytile;
    oldoriginmap = originmap;
    oldpanadjust = panadjust;
    oldpanx = panx;

    RFL_CalcOriginStuff((long)originxglobal + x, (long)originyglobal + y);

    deltax = originxtile - oldxt;
    absdx = abs(deltax);
    deltay = originytile - oldyt;
    absdy = abs(deltay);

    if (absdx > 1 || absdy > 1) {
        RF_NewPosition(originxglobal, originyglobal);
        return;
    }

    if (!absdx && !absdy)
        return;

    screenmove = deltay * 16 * SCREENWIDTH + deltax * TILEWIDTH;
    for (i = 0; i < 3; i++) {
        screenstart[i] += screenmove;
        if (screenstart[i] > (0x10000 - SCREENSPACE)) {
            screencopy = screenmove > 0 ? FREEEGAMEM : -FREEEGAMEM;
            oldscreen = screenstart[i] - screenmove;
            newscreen = oldscreen + screencopy;
            screenstart[i] = newscreen + screenmove;
            VW_ScreenToScreen(oldscreen, newscreen,
                PORTTILESWIDE * 2, PORTTILESHIGH * 16);
            if (i == screenpage)
                VW_SetScreen(newscreen + oldpanadjust, oldpanx & xpanmask);
        }
    }
    bufferofs = screenstart[otherpage];
    displayofs = screenstart[screenpage];
    masterofs = screenstart[2];

    move = deltax;
    if (deltay == 1) move += UPDATEWIDE;
    else if (deltay == -1) move -= UPDATEWIDE;

    updatestart[0] += move;
    updatestart[1] += move;

    if (deltax) {
        if (deltax == 1) {
            RFL_NewRow(1);
            RFL_OldRow(oldoriginmap, PORTTILESHIGH, mapbyteswide);
            RFL_RemoveAnimsOnX(originxtile - 1);
        } else {
            RFL_NewRow(3);
            RFL_OldRow(oldoriginmap + (PORTTILESWIDE - 1) * 2,
                PORTTILESHIGH, mapbyteswide);
            RFL_RemoveAnimsOnX(originxtile + PORTTILESWIDE);
        }

        update0 = updatestart[0] + PORTTILESWIDE;
        update1 = updatestart[1] + PORTTILESWIDE;
        for (yy = 0; yy < PORTTILESHIGH; yy++) {
            *update0 = *update1 = 0;
            update0 += UPDATEWIDE;
            update1 += UPDATEWIDE;
        }
    }

    if (deltay) {
        if (deltay == 1) {
            RFL_NewRow(2);
            RFL_OldRow(oldoriginmap, PORTTILESWIDE, 2);
            updatespot = UPDATEWIDE * (PORTTILESHIGH - 1);
            RFL_RemoveAnimsOnY(originytile - 1);
        } else {
            RFL_NewRow(0);
            RFL_OldRow(oldoriginmap + mapbwidthtable[PORTTILESHIGH - 1],
                PORTTILESWIDE, 2);
            updatespot = 0;
            RFL_RemoveAnimsOnY(originytile + PORTTILESHIGH);
        }
        *(updatestart[0] + updatespot + PORTTILESWIDE) =
        *(updatestart[1] + updatespot + PORTTILESWIDE) = 0;
    }

    update0 = updatestart[0] + UPDATEWIDE * PORTTILESHIGH - 1;
    update1 = updatestart[1] + UPDATEWIDE * PORTTILESHIGH - 1;
    *update0++ = *update1++ = 0;
    *(unsigned *)update0 = *(unsigned *)update1 = UPDATETERMINATE;
}

void RF_PlaceSprite(void **user, unsigned globalx, unsigned globaly,
    unsigned spritenumber, drawtype draw, int priority)
{
    spritelisttype *sprite, *next;
    spritetabletype *spr;
    spritetype *block;
    unsigned shift, pixx;

    if (!spritenumber) {
        RF_RemoveSprite(user);
        return;
    }

    sprite = (spritelisttype *)*user;

    if (sprite) {
        if (sprite->updatecount < 2) {
            if (!sprite->updatecount)
                memcpy(eraselistptr[otherpage]++, sprite, sizeof(eraseblocktype));
            memcpy(eraselistptr[screenpage]++, sprite, sizeof(eraseblocktype));
        }
        if (priority != sprite->priority) {
            next = sprite->nextsprite;
            if (next)
                next->prevptr = sprite->prevptr;
            *sprite->prevptr = next;
            goto linknewspot;
        }
    } else {
        if (!spritefreeptr)
            Quit("RF_PlaceSprite: No free spots in spritearray!");
        sprite = spritefreeptr;
        spritefreeptr = spritefreeptr->nextsprite;

linknewspot:
        next = prioritystart[priority];
        if (next)
            next->prevptr = &sprite->nextsprite;
        sprite->nextsprite = next;
        prioritystart[priority] = sprite;
        sprite->prevptr = &prioritystart[priority];
    }

    spr = &spritetable[spritenumber - STARTSPRITES];
    block = (spritetype *)grsegs[spritenumber];

    globaly += spr->orgy;
    globalx += spr->orgx;

    pixx = globalx >> G_SY_SHIFT;
    shift = (pixx & 7) / 2;

    sprite->screenx = pixx >> (G_EGASX_SHIFT - G_SY_SHIFT);
    sprite->screeny = globaly >> G_SY_SHIFT;
    sprite->width = block->width[shift];
    sprite->height = spr->height;
    sprite->grseg = spritenumber;
    sprite->sourceofs = block->sourceoffset[shift];
    sprite->planesize = block->planesize[shift];
    sprite->draw = draw;
    sprite->priority = priority;
    sprite->tilex = sprite->screenx >> SX_T_SHIFT;
    sprite->tiley = sprite->screeny >> SY_T_SHIFT;
    sprite->tilewide = ((sprite->screenx + sprite->width - 1) >> SX_T_SHIFT)
        - sprite->tilex + 1;
    sprite->tilehigh = ((sprite->screeny + sprite->height - 1) >> SY_T_SHIFT)
        - sprite->tiley + 1;

    sprite->updatecount = 2;
    *user = sprite;
}

void RF_RemoveSprite(void **user)
{
    spritelisttype *sprite, *next;

    sprite = (spritelisttype *)*user;
    if (!sprite) return;

    if (sprite->updatecount < 2) {
        if (!sprite->updatecount)
            memcpy(eraselistptr[otherpage]++, sprite, sizeof(eraseblocktype));
        memcpy(eraselistptr[screenpage]++, sprite, sizeof(eraseblocktype));
    }

    next = sprite->nextsprite;
    if (next)
        next->prevptr = sprite->prevptr;
    *sprite->prevptr = next;

    sprite->nextsprite = spritefreeptr;
    spritefreeptr = sprite;

    *user = 0;
}

void RFL_EraseBlocks(void)
{
    eraseblocktype *block, *done;
    unsigned pos, xtl, ytl, xth, yth, x, y;
    int screenxh, screenyh;
    byte *updatespot;
    unsigned updatedelta;

    block = otherpage ? &eraselist[1][0] : &eraselist[0][0];
    done = eraselistptr[otherpage];

    while (block != done) {
        block->screenx -= originxscreen;
        block->screeny -= originyscreen;

        if (block->screenx < 0) {
            block->width += block->screenx;
            if (block->width < 1) goto next;
            block->screenx = 0;
        }
        if (block->screeny < 0) {
            block->height += block->screeny;
            if (block->height < 1) goto next;
            block->screeny = 0;
        }

        screenxh = block->screenx + block->width;
        screenyh = block->screeny + block->height;

        if (screenxh > EGAPORTSCREENWIDE) {
            block->width = EGAPORTSCREENWIDE - block->screenx;
            screenxh = block->screenx + block->width;
        }
        if (screenyh > PORTSCREENHIGH) {
            block->height = PORTSCREENHIGH - block->screeny;
            screenyh = block->screeny + block->height;
        }
        if (block->width < 1 || block->height < 1) goto next;

        pos = ylookup[block->screeny] + block->screenx;
        VW_ScreenToScreen(masterofs + pos, bufferofs + pos,
            block->width, block->height);

        xtl = block->screenx >> SX_T_SHIFT;
        xth = (block->screenx + block->width - 1) >> SX_T_SHIFT;
        ytl = block->screeny >> SY_T_SHIFT;
        yth = (block->screeny + block->height - 1) >> SY_T_SHIFT;

        updatespot = updateptr + uwidthtable[ytl] + xtl;
        updatedelta = UPDATEWIDE - (xth - xtl + 1);

        for (y = ytl; y <= yth; y++) {
            for (x = xtl; x <= xth; x++)
                *updatespot++ = 2;
            updatespot += updatedelta;
        }

next:
        block++;
    }
    eraselistptr[otherpage] = otherpage ? &eraselist[1][0] : &eraselist[0][0];
}

void RFL_UpdateSprites(void)
{
    spritelisttype *sprite;
    int portx, porty, x, y, xtl, xth, ytl, yth;
    int priority;
    unsigned dest;
    byte *updatespot, *baseupdatespot;
    unsigned updatedelta;
    unsigned height, sourceofs;

    for (priority = 0; priority < PRIORITIES; priority++) {
        if (priority == MASKEDTILEPRIORITY)
            RFL_MaskForegroundTiles();

        for (sprite = prioritystart[priority]; sprite;
             sprite = (spritelisttype *)sprite->nextsprite) {

            portx = sprite->screenx - originxscreen;
            porty = sprite->screeny - originyscreen;
            xtl = portx >> SX_T_SHIFT;
            xth = (portx + sprite->width - 1) >> SX_T_SHIFT;
            ytl = porty >> SY_T_SHIFT;
            yth = (porty + sprite->height - 1) >> SY_T_SHIFT;

            if (xtl < 0) xtl = 0;
            if (xth >= PORTTILESWIDE) xth = PORTTILESWIDE - 1;
            if (ytl < 0) ytl = 0;
            if (yth >= PORTTILESHIGH) yth = PORTTILESHIGH - 1;

            if (xtl > xth || ytl > yth) continue;

            updatespot = baseupdatespot = updateptr + uwidthtable[ytl] + xtl;
            updatedelta = UPDATEWIDE - (xth - xtl + 1);

            if (sprite->updatecount) {
                sprite->updatecount--;
                goto redraw;
            }

            for (y = ytl; y <= yth; y++) {
                for (x = xtl; x <= xth; x++)
                    if (*updatespot++)
                        goto redraw;
                updatespot += updatedelta;
            }
            continue;

redraw:
            updatespot = baseupdatespot;
            for (y = ytl; y <= yth; y++) {
                for (x = xtl; x <= xth; x++)
                    *updatespot++ = 3;
                updatespot += updatedelta;
            }

            height = sprite->height;
            sourceofs = sprite->sourceofs;
            if (porty < 0) {
                height += porty;
                sourceofs -= porty * sprite->width;
                porty = 0;
            } else if (porty + height > PORTSCREENHIGH) {
                height = PORTSCREENHIGH - porty;
            }

            dest = bufferofs + ylookup[porty] + portx;

            if (sprite->draw == spritedraw) {
                VW_MaskBlock(grsegs[sprite->grseg], sourceofs,
                    dest, sprite->width, height, sprite->planesize);
            }
        }
    }
}

void RF_Refresh(void)
{
    byte *newupdate;
    long newtime;

    updateptr = updatestart[otherpage];

    RFL_AnimateTiles();
    RFL_UpdateTiles();
    RFL_EraseBlocks();
    RFL_UpdateSprites();

    if (refreshvector)
        refreshvector();

    VW_SetScreen(bufferofs + panadjust, panx & xpanmask);

    updatestart[otherpage] = newupdate = baseupdatestart[otherpage];
    memset(newupdate, 0, UPDATESCREENSIZE - 2);
    *(unsigned *)(newupdate + UPDATESCREENSIZE - 2) = UPDATETERMINATE;

    screenpage ^= 1;
    otherpage ^= 1;
    bufferofs = screenstart[otherpage];
    displayofs = screenstart[screenpage];

    if (lasttimecount > TimeCount)
        lasttimecount = TimeCount;
    do {
        newtime = TimeCount;
        tics = newtime - lasttimecount;
    } while (tics < MINTICS);
    lasttimecount = newtime;

    if (tics > MAXTICS) {
        TimeCount -= (tics - MAXTICS);
        tics = MAXTICS;
    }
}

void RFL_NewTile(unsigned updateoffset)
{
    unsigned mapoffset;
    unsigned *bg, *fg;
    unsigned tile;
    int i, j, p, bit;
    byte c, mask;

    mapoffset = updatemapofs[updateoffset];
    bg = mapsegs[0] + (mapoffset / 2);
    fg = mapsegs[1] + (mapoffset / 2);

    tile = *bg;
    if (tile < NUMTILE16) {
        if (!tilecache[tile]) {
            tilecache[tile] = 1;
        }
        {
            byte *src = (byte *)grsegs[STARTTILE16 + tile];
            unsigned dest = masterofs + blockstarts[updateoffset];
            unsigned dst_x = (dest - masterofs) % SCREENWIDTH;
            unsigned dst_y = ((dest - masterofs) / SCREENWIDTH);

            for (j = 0; j < 16; j++) {
                for (i = 0; i < 2; i++) {
                    c = 0;
                    mask = 0x80;
                    for (bit = 0; bit < 8; bit++) {
                        for (p = 0; p < 4; p++) {
                            if (src[p * 2 * 16 + j * 2 + i] & mask)
                                c |= (1 << p);
                        }
                        mask >>= 1;
                    }
                    rtg.framebuffer[(dst_y + j) * rtg.bytes_per_row
                        + dst_x * 8 + i * 8 + bit] = c;
                }
            }
        }
    }

    {
        byte *src = (byte *)grsegs[STARTTILE16M + *fg];
        unsigned dest = masterofs + blockstarts[updateoffset];
        unsigned dst_x = (dest - masterofs) % SCREENWIDTH;
        unsigned dst_y = ((dest - masterofs) / SCREENWIDTH);
        int w = 2;

        for (j = 0; j < 16; j++) {
            for (i = 0; i < w; i++) {
                c = 0;
                mask = 0x80;
                for (bit = 0; bit < 8; bit++) {
                    for (p = 0; p < 4; p++) {
                        if (src[p * w * 16 + j * w + i] & mask)
                            c |= (1 << p);
                    }
                    mask >>= 1;
                }
                rtg.framebuffer[(dst_y + j) * rtg.bytes_per_row
                    + dst_x * 8 + i * 8 + bit] = c;
            }
        }
    }
}

void RFL_UpdateTiles(void)
{
    unsigned *bg, *fg;
    unsigned tile, mapoffset;
    int x, y, p, bit;
    byte c, mask;
    unsigned dest;
    byte *upd, *end;
    int src_x, src_y;

    upd = updateptr;
    end = upd + UPDATEWIDE * PORTTILESHIGH;

    for (y = 0; y < PORTTILESHIGH; y++) {
        for (x = 0; x < PORTTILESWIDE; x++) {
            if (*upd) {
                mapoffset = updatemapofs[y * UPDATEWIDE + x];
                bg = mapsegs[0] + (mapoffset / 2);
                tile = *bg;

                if (tile < NUMTILE16) {
                    byte *src = (byte *)grsegs[STARTTILE16 + tile];
                    int ii, jj;
                    dest = masterofs + blockstarts[y * UPDATEWIDE + x];
                    unsigned dst_x = (dest - masterofs) % SCREENWIDTH;
                    unsigned dst_y = ((dest - masterofs) / SCREENWIDTH);

                    for (jj = 0; jj < 16; jj++) {
                        for (ii = 0; ii < 2; ii++) {
                            c = 0;
                            mask = 0x80;
                            for (bit = 0; bit < 8; bit++) {
                                for (p = 0; p < 4; p++) {
                                    if (src[p * 2 * 16 + jj * 2 + ii] & mask)
                                        c |= (1 << p);
                                }
                                mask >>= 1;
                            }
                            if (c) {
                                rtg.framebuffer[(dst_y + jj) * rtg.bytes_per_row
                                    + dst_x * 8 + ii * 8 + bit] = c;
                            }
                        }
                    }
                }

                fg = mapsegs[1] + (mapoffset / 2);
                tile = *fg;
                if (tile < NUMTILE16M) {
                    byte *src = (byte *)grsegs[STARTTILE16M + tile];
                    int ii, jj;
                    dest = masterofs + blockstarts[y * UPDATEWIDE + x];
                    unsigned dst_x = (dest - masterofs) % SCREENWIDTH;
                    unsigned dst_y = ((dest - masterofs) / SCREENWIDTH);

                    for (jj = 0; jj < 16; jj++) {
                        for (ii = 0; ii < 2; ii++) {
                            c = 0;
                            mask = 0x80;
                            for (bit = 0; bit < 8; bit++) {
                                for (p = 0; p < 4; p++) {
                                    if (src[p * 2 * 16 + jj * 2 + ii] & mask)
                                        c |= (1 << p);
                                }
                                mask >>= 1;
                            }
                            if (c) {
                                rtg.framebuffer[(dst_y + jj) * rtg.bytes_per_row
                                    + dst_x * 8 + ii * 8 + bit] = c;
                            }
                        }
                    }
                }

                *upd = 0;
            }
            upd++;
        }
        upd++;
    }
}

void VWL_UpdateScreenBlocks(void)
{
    RFL_UpdateTiles();
}

void RFL_MaskForegroundTiles(void)
{
}
