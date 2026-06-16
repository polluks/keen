#include "id_heads.h"
#include <stdlib.h>

#define MAXBLOCKS 1300

typedef struct {
    memptr ptr;
    unsigned long size;
    int purge;
    int locked;
} memblock_t;

static memblock_t blocks[MAXBLOCKS];
static int numblocks;
static unsigned long total_allocated;

mminfotype mminfo;
memptr bufferseg;
boolean bombonerror = true;
void (*beforesort)(void) = NULL;
void (*aftersort)(void) = NULL;

void MM_Startup(void)
{
    numblocks = 0;
    total_allocated = 0;
    mminfo.mainmem = 1024 * 1024;
    mminfo.nearheap = 0;
    mminfo.farheap = 0;
    mminfo.EMSmem = 0;
    mminfo.XMSmem = 0;
}

void MM_Shutdown(void)
{
    int i;
    for (i = 0; i < numblocks; i++) {
        if (blocks[i].ptr)
            free(blocks[i].ptr);
    }
    numblocks = 0;
}

static memblock_t *find_block(memptr ptr)
{
    int i;
    for (i = 0; i < numblocks; i++) {
        if (blocks[i].ptr == ptr)
            return &blocks[i];
    }
    return NULL;
}

void MM_GetPtr(memptr *baseptr, unsigned long size)
{
    if (numblocks >= MAXBLOCKS)
        Quit("MM_GetPtr: out of block descriptors!");

    blocks[numblocks].ptr = malloc(size);
    if (!blocks[numblocks].ptr) {
        if (bombonerror)
            Quit("MM_GetPtr: out of memory!");
        *baseptr = NULL;
        return;
    }
    memset(blocks[numblocks].ptr, 0, size);
    blocks[numblocks].size = size;
    blocks[numblocks].purge = 0;
    blocks[numblocks].locked = 0;
    total_allocated += size;
    *baseptr = blocks[numblocks].ptr;
    numblocks++;
}

void MM_FreePtr(memptr *baseptr)
{
    int i;
    if (!*baseptr) return;

    for (i = 0; i < numblocks; i++) {
        if (blocks[i].ptr == *baseptr) {
            free(blocks[i].ptr);
            total_allocated -= blocks[i].size;
            blocks[i] = blocks[numblocks - 1];
            numblocks--;
            *baseptr = NULL;
            return;
        }
    }
}

void MM_SetPurge(memptr *baseptr, int purge)
{
    memblock_t *b = find_block(*baseptr);
    if (b) b->purge = purge;
}

void MM_SetLock(memptr *baseptr, boolean locked)
{
    memblock_t *b = find_block(*baseptr);
    if (b) b->locked = locked;
}

void MM_SortMem(void)
{
    if (beforesort) beforesort();
    if (aftersort) aftersort();
}

void MM_ShowMemory(void) {}

long MM_UnusedMemory(void)
{
    return 0;
}

long MM_TotalFree(void)
{
    return 0;
}
