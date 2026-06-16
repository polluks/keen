#ifndef __AMIGA_DIR_WRAPPER__
#define __AMIGA_DIR_WRAPPER__

#include <dos/dos.h>
#include <proto/dos.h>

struct ffblk {
    char ff_name[256];
};

static __inline int findfirst(const char *name, struct ffblk *ffblk, int attr)
{
    (void)name;
    (void)ffblk;
    (void)attr;
    return -1;
}

static __inline int findnext(struct ffblk *ffblk)
{
    (void)ffblk;
    return -1;
}

#endif
