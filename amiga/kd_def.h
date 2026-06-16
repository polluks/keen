#ifndef __KD_DEF__
#define __KD_DEF__

#include "id_heads.h"
#include "bios.h"
#include "soft.h"
#include "sl_file.h"

#define FRILLS 0

#define CREDITS 0
#define MAXACTORS MAXSPRITES

#define ACCGRAVITY 3
#define SPDMAXY    80
#define BLOCKSIZE  (8*PIXGLOBAL)

#define SCROLLEAST  (TILEGLOBAL*11)
#define SCROLLWEST  (TILEGLOBAL*9)
#define SCROLLSOUTH (TILEGLOBAL*8)
#define SCROLLNORTH (TILEGLOBAL*4)

#define CLIPMOVE 24
#define GAMELEVELS 17

typedef enum { notdone, resetgame, levelcomplete, warptolevel, died, victorious } exittype;
typedef enum { nothing, keenobj, powerobj, doorobj,
    bonusobj, broccoobj, tomatobj, carrotobj, celeryobj, asparobj, grapeobj,
    taterobj, cartobj, frenchyobj, melonobj, turnipobj, cauliobj, brusselobj,
    mushroomobj, squashobj, apelobj, peapodobj, peabrainobj, boobusobj,
    shotobj, inertobj } classtype;

typedef struct {
    int leftshapenum, rightshapenum;
    enum { step, slide, think, stepthink, slidethink } progress;
    boolean skippable;
} animtile;

#define MAX_UPDATE_SPRITES   50
#define MAX_UPDATE_TILES     50

#endif
