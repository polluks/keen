#ifndef __KD_DEF__
#define __KD_DEF__

#include "id_heads.h"
#include "bios.h"
#include "soft.h"
#include "sl_file.h"
#include "gelib.h"

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

typedef struct {
    int leftshapenum, rightshapenum;
    enum { step, slide, think, stepthink, slidethink } progress;
    boolean skippable;
    boolean pushtofloor;
    int tictime;
    int xmove;
    int ymove;
    void (*think)(void);
    void (*contact)(void);
    void (*react)(void);
    void *nextstate;
} statetype;

typedef struct {
    unsigned worldx, worldy;
    boolean leveldone[GAMELEVELS];
    long score, nextextra;
    int flowerpowers;
    int boobusbombs, bombsthislevel;
    int keys;
    int mapon;
    int lives;
    int difficulty;
} gametype;

typedef struct objstruct {
    classtype obclass;
    enum { no, yes, allways, removable } active;
    boolean needtoreact, needtoclip;
    unsigned nothink;
    unsigned x, y;
    int xdir, ydir;
    int xmove, ymove;
    int xspeed, yspeed;
    int ticcount, ticadjust;
    statetype *state;
    unsigned shapenum;
    unsigned left, top, right, bottom;
    unsigned midx;
    unsigned tileleft, tiletop, tileright, tilebottom;
    unsigned tilemidx;
    int hitnorth, hiteast, hitsouth, hitwest;
    int temp1, temp2, temp3, temp4;
    void *sprite;
    struct objstruct *next, *prev;
} objtype;

typedef struct {
    int handle;
    memptr buffer;
    word offset;
    word status;
} BufferedIO;

#define MAX_UPDATE_SPRITES   50
#define MAX_UPDATE_TILES     50

extern char str[80], str2[20];
extern boolean singlestep, jumpcheat, godmode, tedlevel;
extern unsigned tedlevelnum;

void DebugMemory(void);
void TestSprites(void);
int DebugKeys(void);
void StartupId(void);
void ShutdownId(void);
void InitGame(void);

void Finale(void);
void GameOver(void);
void DemoLoop(void);
void StatusWindow(void);
void NewGame(void);
void TEDDeath(void);
boolean LoadGame(int file);
boolean SaveGame(int file);
void ResetGame(void);

extern gametype gamestate;
extern exittype playstate;
extern boolean button0held, button1held;
extern unsigned originxtilemax, originytilemax;
extern objtype *new, *check, *player, *scoreobj;

extern objtype dummyobj;

extern char *levelnames[21];

void CheckKeys(void);
void CalcInactivate(void);
void InitObjArray(void);
void GetNewObj(boolean usedummy);
void RemoveObj(objtype *gone);
void ScanInfoPlane(void);
void PatchWorldMap(void);
void MarkTileGraphics(void);
void FadeAndUnhook(void);
void SetupGameLevel(boolean loadnow);
void ScrollScreen(void);
void MoveObjVert(objtype *ob, int ymove);
void MoveObjHoriz(objtype *ob, int xmove);
void GivePoints(unsigned points);
void ClipToEnds(objtype *ob);
void ClipToEastWalls(objtype *ob);
void ClipToWestWalls(objtype *ob);
void ClipToWalls(objtype *ob);
void ClipToSprite(objtype *push, objtype *solid, boolean squish);
void ClipToSpriteSide(objtype *push, objtype *solid);
int DoActor(objtype *ob, int tics);
void StateMachine(objtype *ob);
void NewState(objtype *ob, statetype *state);
void PlayLoop(void);
void GameLoop(void);

void CalcSingleGravity(void);
void ProjectileThink(objtype *ob);
void VelocityThink(objtype *ob);
void DrawReact(objtype *ob);
void SpawnScore(void);
void FixScoreBox(void);
void SpawnWorldKeen(int tilex, int tiley);
void SpawnKeen(int tilex, int tiley, int dir);
void KillKeen(void);

extern int singlegravity;
extern unsigned bounceangle[8][8];
extern statetype s_keendie1;

void WalkReact(objtype *ob);
void DoGravity(objtype *ob);
void AccelerateX(objtype *ob, int dir, int max);
void FrictionX(objtype *ob);
void DrawReact2(objtype *ob);
void DrawReact3(objtype *ob);
void ChangeState(objtype *ob, statetype *state);
void ChangeToFlower(objtype *ob);
void SpawnBonus(int tilex, int tiley, int type);
void SpawnDoor(int tilex, int tiley);
void SpawnBrocco(int tilex, int tiley);
void SpawnTomat(int tilex, int tiley);
void SpawnCarrot(int tilex, int tiley);
void SpawnAspar(int tilex, int tiley);
void SpawnGrape(int tilex, int tiley);

extern statetype s_doorraise;
extern statetype s_bonus;
extern statetype s_bonusrise;
extern statetype s_broccosmash3;
extern statetype s_broccosmash4;
extern statetype s_grapefall;

void SpawnTater(int tilex, int tiley);
void SpawnCart(int tilex, int tiley);
void SpawnFrenchy(int tilex, int tiley);
void SpawnMelon(int tilex, int tiley, int dir);
void SpawnSquasher(int tilex, int tiley);
void SpawnApel(int tilex, int tiley);
void SpawnPeaPod(int tilex, int tiley);
void SpawnPeaBrain(int tilex, int tiley);
void SpawnBoobus(int tilex, int tiley);

extern statetype s_taterattack2;
extern statetype s_squasherjump2;
extern statetype s_boobusdie;
extern statetype s_deathwait1;
extern statetype s_deathwait2;
extern statetype s_deathwait3;
extern statetype s_deathboom1;
extern statetype s_deathboom2;

void FreeShape(struct Shape *shape);
int UnpackEGAShapeToScreen(struct Shape *SHP, int startx, int starty);
long Verify(char *filename);
memptr InitBufferedIO(int handle, BufferedIO *bio);
void FreeBufferedIO(BufferedIO *bio);
byte bio_readch(BufferedIO *bio);
void bio_fillbuffer(BufferedIO *bio);
void SwapLong(long far *Var);
void SwapWord(unsigned int far *Var);
void MoveGfxDst(short x, short y);

#endif
