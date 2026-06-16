#ifndef __ID_SD__
#define __ID_SD__

#include "id_heads.h"

#define TickBase 70

typedef enum { sdm_Off, sdm_PC, sdm_AdLib, sdm_SoundBlaster, sdm_SoundSource } SDMode;
typedef enum { smm_Off, smm_AdLib } SMMode;

typedef struct {
    longword length;
    word priority;
} SoundCommon;

typedef struct {
    SoundCommon common;
    byte data[1];
} PCSound;

typedef struct {
    SoundCommon common;
    word hertz;
    byte bits, reference, data[1];
} SampledSound;

typedef struct {
    byte mChar, cChar, mScale, cScale, mAttack, cAttack, mSus, cSus, mWave, cWave, nConn;
    byte unused[5];
} Instrument;

typedef struct {
    SoundCommon common;
    Instrument inst;
    byte block, data[1];
} AdLibSound;

#define sqMaxTracks 10
#define sqMaxMoods  1

#define sev_Null      0
#define sev_NoteOff   1
#define sev_NoteOn    2
#define sev_NotePitch 3
#define sev_NewInst   4
#define sev_NewPerc   5
#define sev_PercOn    6
#define sev_PercOff   7
#define sev_SeqEnd    -1

typedef struct {
    word flags, count, offsets[1];
} MusicGroup;

typedef struct {
    word mood, *moods[sqMaxMoods];
    Instrument inst;
    word *seq;
    longword nextevent;
} ActiveTrack;

#define sqmode_Normal  0
#define sqmode_FadeIn  1
#define sqmode_FadeOut 2
#define sqMaxFade      64

extern boolean LeaveDriveOn;
extern boolean SoundSourcePresent, SoundBlasterPresent, AdLibPresent;
extern boolean NeedsDigitized, NeedsMusic;
extern SDMode SoundMode;
extern SMMode MusicMode;
extern longword TimeCount;

extern boolean ssIsTandy;
extern word ssPort;

extern void SD_Startup(void), SD_Shutdown(void);
extern void SD_Default(boolean gotit, SDMode sd, SMMode sm);
extern void SD_PlaySound(word sound), SD_StopSound(void);
extern void SD_WaitSoundDone(void);
extern void SD_StartMusic(Ptr music), SD_FadeOutMusic(void);
extern void SD_SetUserHook(void (*hook)(void));
extern boolean SD_MusicPlaying(void);
extern boolean SD_SetSoundMode(SDMode mode);
extern boolean SD_SetMusicMode(SMMode mode);
extern word SD_SoundPlaying(void);

#endif
