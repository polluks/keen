#ifndef __ID_MM__
#define __ID_MM__

#include "id_heads.h"

#define SAVENEARHEAP  0x400
#define SAVEFARHEAP   0
#define BUFFERSIZE    0x1000
#define MAXBLOCKS     1300

typedef void *memptr;

typedef struct {
    long nearheap, farheap, EMSmem, XMSmem, mainmem;
} mminfotype;

extern mminfotype mminfo;
extern memptr bufferseg;
extern boolean bombonerror;
extern void (*beforesort)(void);
extern void (*aftersort)(void);

void MM_Startup(void);
void MM_Shutdown(void);
void MM_GetPtr(memptr *baseptr, unsigned long size);
void MM_FreePtr(memptr *baseptr);
void MM_SetPurge(memptr *baseptr, int purge);
void MM_SetLock(memptr *baseptr, boolean locked);
void MM_SortMem(void);
void MM_ShowMemory(void);
long MM_UnusedMemory(void);
long MM_TotalFree(void);

#endif
