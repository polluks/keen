#ifndef __AMIGA_DOS_WRAPPER__
#define __AMIGA_DOS_WRAPPER__

#include <dos/dos.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define O_RDONLY  MODE_OLDFILE
#define O_BINARY  0
#define O_WRONLY  MODE_NEWFILE
#define O_CREAT   0
#define O_TRUNC   0

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

static __inline int open(const char *name, int mode, ...)
{
    BPTR fh;
    if (mode & O_WRONLY)
        fh = Open((STRPTR)name, MODE_NEWFILE);
    else
        fh = Open((STRPTR)name, MODE_OLDFILE);
    return (int)fh;
}

static __inline int close(int fd)
{
    return Close((BPTR)fd) ? 0 : -1;
}

static __inline long lseek(int fd, long offset, int whence)
{
    return Seek((BPTR)fd, offset, (long)whence);
}

static __inline int read(int fd, void *buf, int len)
{
    return Read((BPTR)fd, buf, (long)len);
}

static __inline int write(int fd, const void *buf, int len)
{
    return Write((BPTR)fd, (void *)buf, (long)len);
}

static __inline long filelength(int fd)
{
    long pos, size;
    pos = Seek((BPTR)fd, 0, OFFSET_CURRENT);
    size = Seek((BPTR)fd, 0, OFFSET_END);
    Seek((BPTR)fd, pos, OFFSET_BEGINNING);
    return size;
}

#endif
