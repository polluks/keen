#ifndef __SOFT_H__
#define __SOFT_H__

#include "jam_io.h"

unsigned long BLoad(char *SourceFile, memptr *DstPtr);
memptr LoadLIBFile(char *LibName, char *FileName, memptr *MemPtr);
int LoadLIBShape(char *SLIB_Filename, char *Filename, struct Shape *SHP);

#endif
