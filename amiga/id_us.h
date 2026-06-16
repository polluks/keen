#ifndef __ID_US__
#define __ID_US__

#include "id_heads.h"

extern int _argc;
extern char **_argv;

void US_Startup(void);
void US_Shutdown(void);
int  US_CheckParm(char *parm, char *strings[]);
void US_Print(char *s);
void US_CPrint(char *s);
void US_PrintUnsigned(longword n);
void US_PrintSigned(long n);
void US_PrintBig(char *s);

void US_CenterWindow(int w, int h);
void US_ExitWindow(void);
void US_ClearWindow(void);
void US_DrawWindow(int x, int y, int w, int h);
void US_UpdateCursor(int x, int y);

#endif
