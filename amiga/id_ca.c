#include "id_heads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>

typedef struct {
    unsigned bit0, bit1;
} huffnode;

static huffnode audiohuffman[255];
static long *audiostarts;
static int audiohandle = -1;

byte  *tinf;
int   mapon;
unsigned *mapsegs[3];
maptype *mapheaderseg[NUMMAPS];
byte   *audiosegs[NUMSNDCHUNKS];
void   *grsegs[NUMCHUNKS];
byte   grneeded[NUMCHUNKS];
byte   ca_levelbit, ca_levelnum;
char   *titleptr[8];
int    profilehandle;

static void CAL_OptimizeNodes(huffnode *table)
{
    int i;
    for (i = 0; i < 255; i++) {
        if (table[i].bit0 >= 256)
            table[i].bit0 = (unsigned)(table + (table[i].bit0 - 256));
        if (table[i].bit1 >= 256)
            table[i].bit1 = (unsigned)(table + (table[i].bit1 - 256));
    }
}

static void CAL_HuffExpand(byte *source, byte *dest, long length, huffnode *hufftable)
{
    unsigned bitmask = 1;
    unsigned node;
    huffnode *headptr = hufftable + 254;
    byte *end = dest + length;

    while (dest < end) {
        node = (unsigned)headptr;
        while (node >= 256) {
            if (*source & bitmask)
                node = ((huffnode *)node)->bit1;
            else
                node = ((huffnode *)node)->bit0;
            bitmask <<= 1;
            if (!bitmask) {
                bitmask = 1;
                source++;
            }
        }
        *dest++ = (byte)node;
    }
}

void CAL_ShiftSprite(unsigned segment, unsigned source, unsigned dest,
    unsigned width, unsigned height, unsigned pixshift)
{
}

boolean CA_FarRead(int handle, byte *dest, long length)
{
    return (read(handle, dest, length) == length);
}

boolean CA_FarWrite(int handle, byte *source, long length)
{
    return (write(handle, source, length) == length);
}

boolean CA_LoadFile(char *filename, memptr *ptr)
{
    int handle;
    long size;

    handle = open(filename, O_RDONLY | O_BINARY);
    if (handle == -1) return false;

    size = lseek(handle, 0, SEEK_END);
    lseek(handle, 0, SEEK_SET);

    MM_GetPtr(ptr, size);
    if (!*ptr) {
        close(handle);
        return false;
    }

    if (read(handle, *ptr, size) != size) {
        MM_FreePtr(ptr);
        close(handle);
        return false;
    }

    close(handle);
    return true;
}

long CA_RLEWCompress(unsigned huge *source, long length, unsigned huge *dest,
    unsigned rlewtag)
{
    long srci = 1;
    long dst = 1;
    long runlen;
    unsigned value;
    unsigned i;

    while (srci < length) {
        value = source[srci];
        runlen = 1;
        while (srci + runlen < length && source[srci + runlen] == value && runlen < 65535)
            runlen++;

        if (runlen > 3 || value == rlewtag) {
            dest[dst++] = rlewtag;
            dest[dst++] = runlen;
            dest[dst++] = value;
        } else {
            for (i = 0; i < runlen; i++)
                dest[dst++] = value;
        }
        srci += runlen;
    }
    return dst;
}

void CA_RLEWexpand(unsigned huge *source, unsigned huge *dest, long length,
    unsigned rlewtag)
{
    long srci = 0;
    long dsti = 0;
    unsigned value;
    unsigned count;
    unsigned i;

    while (dsti < length) {
        value = source[srci++];
        if (value != rlewtag) {
            dest[dsti++] = value;
        } else {
            count = source[srci++];
            value = source[srci++];
            for (i = 0; i < count; i++)
                dest[dsti++] = value;
        }
    }
}

void CA_Startup(void)
{
    int i;
    long length;

    mapon = 0;
    for (i = 0; i < NUMCHUNKS; i++) {
        grsegs[i] = NULL;
        grneeded[i] = 0;
    }
    for (i = 0; i < NUMMAPS; i++)
        mapheaderseg[i] = NULL;
    for (i = 0; i < 3; i++)
        mapsegs[i] = NULL;
    ca_levelbit = 1;
    ca_levelnum = 0;

    /* Load audio dictionary (Huffman table) */
    {
        int dhandle = open("AUDIODCT.KDR", O_RDONLY | O_BINARY, S_IREAD);
        if (dhandle != -1) {
            read(dhandle, audiohuffman, sizeof(audiohuffman));
            close(dhandle);
            CAL_OptimizeNodes(audiohuffman);
        }
    }

    /* Load audio header (chunk start offsets) */
    {
        int hhandle = open("AUDIOHHD.KDR", O_RDONLY | O_BINARY, S_IREAD);
        if (hhandle != -1) {
            length = lseek(hhandle, 0, SEEK_END);
            lseek(hhandle, 0, SEEK_SET);
            audiostarts = (long *)malloc(length);
            if (audiostarts)
                read(hhandle, audiostarts, length);
            close(hhandle);
        }
    }

    /* Open audio data file */
    audiohandle = open("KDREAMS.AUD", O_RDONLY | O_BINARY, S_IREAD);
}

void CA_Shutdown(void)
{
    int i;
    for (i = 0; i < NUMCHUNKS; i++) {
        if (grsegs[i])
            MM_FreePtr(&grsegs[i]);
    }
    for (i = 0; i < NUMMAPS; i++) {
        if (mapheaderseg[i])
            MM_FreePtr((memptr *)&mapheaderseg[i]);
    }
    for (i = 0; i < 3; i++) {
        if (mapsegs[i])
            MM_FreePtr((memptr *)&mapsegs[i]);
    }
    if (audiohandle != -1)
        close(audiohandle);
    if (audiostarts)
        free(audiostarts);
}

void CA_CacheGrChunk(int chunk)
{
    if (!grsegs[chunk]) {
        char fname[13];
        sprintf(fname, "%s", "gfx.kdr");
    }
    grneeded[chunk] = 0;
}

void CA_CacheMap(int mapnum)
{
}

void CA_CacheAudioChunk(int chunk)
{
    long pos, compressed, expanded;
    byte *source;
    byte bigbuffer[4096];
    byte *bigbufferseg = NULL;

    if (chunk < 0 || chunk >= NUMSNDCHUNKS)
        return;

    if (audiosegs[chunk]) {
        MM_SetPurge((memptr *)&audiosegs[chunk], 0);
        return;
    }

    if (audiohandle == -1 || !audiostarts)
        return;

    pos = audiostarts[chunk];
    compressed = audiostarts[chunk + 1] - pos;

    lseek(audiohandle, pos, SEEK_SET);

    if (compressed <= (long)sizeof(bigbuffer)) {
        CA_FarRead(audiohandle, bigbuffer, compressed);
        source = bigbuffer;
    } else {
        bigbufferseg = (byte *)malloc(compressed);
        if (!bigbufferseg) return;
        CA_FarRead(audiohandle, bigbufferseg, compressed);
        source = bigbufferseg;
    }

    expanded = *(long *)source;
    source += 4;

    MM_GetPtr((memptr *)&audiosegs[chunk], expanded);
    if (audiosegs[chunk])
        CAL_HuffExpand(source, audiosegs[chunk], expanded, audiohuffman);

    if (bigbufferseg)
        free(bigbufferseg);
}

void CA_LoadAllSounds(void)
{
    unsigned start, i;

    start = STARTDIGISOUNDS;

    for (i = 0; i < NUMSOUNDS; i++)
        CA_CacheAudioChunk(start + i);
}

void CA_UpLevel(void)
{
    ca_levelbit <<= 1;
    ca_levelnum++;
}

void CA_DownLevel(void)
{
    ca_levelbit >>= 1;
    ca_levelnum--;
}

void CA_ClearMarks(void)
{
    memset(grneeded, 0, sizeof(grneeded));
}

void CA_ClearAllMarks(void)
{
    memset(grneeded, 0, sizeof(grneeded));
}

void CA_CacheMarks(char *title, boolean cachedownlevel)
{
}
