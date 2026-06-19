#ifndef __ID_CA__
#define __ID_CA__

#include "id_heads.h"
#include "audiokdr.h"

/* Reading headers/dictionaries from files at runtime */
/* #undef MAPHEADERLINKED */
/* #undef GRHEADERLINKED */
/* #undef AUDIOHEADERLINKED */

#define NUMMAPS 30

#define SPEED     502
#define ANIM      (SPEED+NUMTILE16)
#define NORTHWALL (ANIM+NUMTILE16)
#define EASTWALL  (NORTHWALL+NUMTILE16M)
#define SOUTHWALL (EASTWALL+NUMTILE16M)
#define WESTWALL  (SOUTHWALL+NUMTILE16M)
#define MANIM     (WESTWALL+NUMTILE16M)
#define INTILE    (MANIM+NUMTILE16M)
#define MSPEED    (INTILE+NUMTILE16M)

typedef struct {
    long planestart[3];
    unsigned planelength[3];
    unsigned width, height;
    char name[16];
} maptype;

extern byte  *tinf;
extern int   mapon;
extern unsigned *mapsegs[3];
extern maptype *mapheaderseg[NUMMAPS];
extern byte   *audiosegs[NUMSNDCHUNKS];
extern void   *grsegs[NUMCHUNKS];
extern byte   grneeded[NUMCHUNKS];
extern byte   ca_levelbit, ca_levelnum;
extern char   *titleptr[8];
extern int    profilehandle;

void CAL_ShiftSprite(unsigned segment, unsigned source, unsigned dest,
    unsigned width, unsigned height, unsigned pixshift);

boolean CA_FarRead(int handle, byte *dest, long length);
boolean CA_FarWrite(int handle, byte *source, long length);
boolean CA_LoadFile(char *filename, memptr *ptr);
long CA_RLEWCompress(unsigned huge *source, long length, unsigned huge *dest,
    unsigned rlewtag);
void CA_RLEWexpand(unsigned huge *source, unsigned huge *dest, long length,
    unsigned rlewtag);
void CA_Startup(void);
void CA_Shutdown(void);
void CA_CacheAudioChunk(int chunk);
void CA_LoadAllSounds(void);
void CA_UpLevel(void);
void CA_DownLevel(void);
void CA_ClearMarks(void);
void CA_ClearAllMarks(void);

#define CA_MarkGrChunk(chunk) grneeded[chunk] |= ca_levelbit

void CA_CacheGrChunk(int chunk);
void CA_CacheMap(int mapnum);
void CA_CacheMarks(char *title, boolean cachedownlevel);

#endif
