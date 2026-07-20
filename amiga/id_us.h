#ifndef __ID_US__
#define __ID_US__

#include "id_heads.h"

#define MaxString 128

typedef struct {
    int x, y, w, h, px, py;
} WindowRec;

typedef enum {
    gd_Continue,
    gd_Easy,
    gd_Normal,
    gd_Hard
} GameDiff;

extern int _argc;
extern char **_argv;

extern boolean ingame, abortgame, loadedgame;
extern char *abortprogram;
extern GameDiff restartgame;
extern word PrintX, PrintY;
extern word WindowX, WindowY, WindowW, WindowH;

#define US_HomeWindow() {PrintX = WindowX; PrintY = WindowY;}

void US_Startup(void);
void US_Shutdown(void);
void US_Setup(void);
void US_InitRndT(boolean randomize);
void US_SetLoadSaveHooks(boolean (*load)(int), boolean (*save)(int), void (*reset)(void));
void US_TextScreen(void);
void US_UpdateTextScreen(void);
void US_FinishTextScreen(void);
void US_ControlPanel(void);
int  US_CheckParm(char *parm, char *strings[]);
void US_Print(char *s);
void US_CPrint(char *s);
void US_PrintUnsigned(longword n);
void US_PrintSigned(long n);
void US_PrintBig(char *s);
void US_PrintCentered(char *s);
void US_LineInput(int x, int y, char *buf, char *def, boolean escok, int maxchars, int maxwidth);
void US_DrawWindow(int x, int y, int w, int h);
void US_CenterWindow(int w, int h);
void US_ExitWindow(void);
void US_ClearWindow(void);
void US_UpdateCursor(int x, int y);
void US_SaveWindow(WindowRec *win);
void US_RestoreWindow(WindowRec *win);
void US_DisplayHighScores(void);
void US_CheckHighScore(long score, word other);
int US_RndT(void);

#endif
