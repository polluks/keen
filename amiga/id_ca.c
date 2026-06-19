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

static huffnode grhuffman[255];
static huffnode maphuffman[255];
static huffnode audiohuffman[255];
static long *grstarts;
static long *audiostarts;
static int grhandle = -1;
static int maphandle = -1;
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

static void CAL_SetupGrFile(void)
{
    int hhandle;
    long length;

    {
        int dhandle = open("EGADICT.KDR", O_RDONLY | O_BINARY, S_IREAD);
        if (dhandle != -1) {
            read(dhandle, grhuffman, sizeof(grhuffman));
            close(dhandle);
            CAL_OptimizeNodes(grhuffman);
        }
    }

    hhandle = open("EGAHEAD.KDR", O_RDONLY | O_BINARY, S_IREAD);
    if (hhandle != -1) {
        length = lseek(hhandle, 0, SEEK_END);
        lseek(hhandle, 0, SEEK_SET);
        grstarts = (long *)malloc(length);
        if (grstarts)
            read(hhandle, grstarts, length);
        close(hhandle);
    }

    grhandle = open("KDREAMS.EGA", O_RDONLY | O_BINARY, S_IREAD);

    if (grhandle != -1 && grstarts) {
        long pos, compsize;
        byte *buf;
        int i;

        for (i = 0; i < 3; i++) {
            int chunk = i;
            pos = grstarts[chunk];
            if (pos < 0) continue;
            compsize = grstarts[chunk + 1] - pos;
            if (compsize <= 0) continue;

            lseek(grhandle, pos, SEEK_SET);
            buf = (byte *)malloc(compsize);
            if (!buf) continue;
            CA_FarRead(grhandle, buf, compsize);

            explen = *(long *)buf;
            if (i == 0) {
                MM_GetPtr((memptr *)&pictable, NUMPICS * sizeof(pictabletype));
                if (pictable)
                    CAL_HuffExpand(buf + 4, (byte *)pictable,
                        NUMPICS * sizeof(pictabletype), grhuffman);
            } else if (i == 1) {
                MM_GetPtr((memptr *)&picmtable, NUMPICM * sizeof(pictabletype));
                if (picmtable)
                    CAL_HuffExpand(buf + 4, (byte *)picmtable,
                        NUMPICM * sizeof(pictabletype), grhuffman);
            } else if (i == 2) {
                MM_GetPtr((memptr *)&spritetable, NUMSPRITES * sizeof(spritetabletype));
                if (spritetable)
                    CAL_HuffExpand(buf + 4, (byte *)spritetable,
                        NUMSPRITES * sizeof(spritetabletype), grhuffman);
            }
            free(buf);
        }
    }
}

static void CAL_SetupMapFile(void)
{
    int hhandle;
    long length;

    {
        int dhandle = open("MAPDICT.KDR", O_RDONLY | O_BINARY, S_IREAD);
        if (dhandle != -1) {
            read(dhandle, maphuffman, sizeof(maphuffman));
            close(dhandle);
            CAL_OptimizeNodes(maphuffman);
        }
    }

    hhandle = open("MAPHEAD.KDR", O_RDONLY | O_BINARY, S_IREAD);
    if (hhandle != -1) {
        length = lseek(hhandle, 0, SEEK_END);
        lseek(hhandle, 0, SEEK_SET);
        MM_GetPtr((memptr *)&tinf, length);
        if (tinf)
            CA_FarRead(hhandle, tinf, length);
        close(hhandle);
    }

    maphandle = open("KDREAMS.MAP", O_RDONLY | O_BINARY, S_IREAD);
}

static void CAL_SetupAudioFile(void)
{
    int hhandle;
    long length;

    {
        int dhandle = open("AUDIODCT.KDR", O_RDONLY | O_BINARY, S_IREAD);
        if (dhandle != -1) {
            read(dhandle, audiohuffman, sizeof(audiohuffman));
            close(dhandle);
            CAL_OptimizeNodes(audiohuffman);
        }
    }

    hhandle = open("AUDIOHHD.KDR", O_RDONLY | O_BINARY, S_IREAD);
    if (hhandle != -1) {
        length = lseek(hhandle, 0, SEEK_END);
        lseek(hhandle, 0, SEEK_SET);
        audiostarts = (long *)malloc(length);
        if (audiostarts)
            read(hhandle, audiostarts, length);
        close(hhandle);
    }

    audiohandle = open("KDREAMS.AUD", O_RDONLY | O_BINARY, S_IREAD);
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

    mapon = -1;
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

    CAL_SetupGrFile();
    CAL_SetupMapFile();
    CAL_SetupAudioFile();
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
    if (tinf)
        MM_FreePtr((memptr *)&tinf);
    if (grhandle != -1)
        close(grhandle);
    if (maphandle != -1)
        close(maphandle);
    if (audiohandle != -1)
        close(audiohandle);
    if (grstarts)
        free(grstarts);
    if (audiostarts)
        free(audiostarts);
    if (pictable)
        MM_FreePtr((memptr *)&pictable);
    if (picmtable)
        MM_FreePtr((memptr *)&picmtable);
    if (spritetable)
        MM_FreePtr((memptr *)&spritetable);
}

static void CAL_CacheSprite(int chunk, byte *compressed)
{
    spritetabletype *spr;
    spritetype *dest;
    unsigned smallplane, bigplane;
    unsigned shiftstarts[MAXSHIFTS + 1];
    unsigned expanded;
    int i;

    spr = &spritetable[chunk - STARTSPRITES];
    smallplane = spr->width * spr->height;
    bigplane = (spr->width + 1) * spr->height;

    shiftstarts[0] = 0;
    for (i = 1; i <= spr->shifts; i++)
        shiftstarts[i] = shiftstarts[i - 1] + smallplane * 5;

    expanded = shiftstarts[spr->shifts];

    MM_GetPtr((memptr *)&grsegs[chunk], expanded + sizeof(spritetype));
    if (!grsegs[chunk]) return;

    dest = (spritetype *)grsegs[chunk];
    for (i = 0; i < MAXSHIFTS; i++) {
        if (i < spr->shifts) {
            dest->sourceoffset[i] = shiftstarts[i];
            dest->planesize[i] = smallplane;
            dest->width[i] = spr->width + i;
        } else {
            dest->sourceoffset[i] = 0;
            dest->planesize[i] = 0;
            dest->width[i] = 0;
        }
    }

    CAL_HuffExpand(compressed, dest->data, smallplane * 5, grhuffman);
}

static void CAL_ExpandGrChunk(int chunk, byte *source)
{
    long expanded;

    if (chunk >= STARTTILE8 && chunk < STARTTILE8 + NUMTILE8) {
        expanded = 32;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTTILE8M && chunk < STARTTILE8M + NUMTILE8M) {
        expanded = 40;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTTILE16 && chunk < STARTTILE16 + NUMTILE16) {
        expanded = 128;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTTILE16M && chunk < STARTTILE16M + NUMTILE16M) {
        expanded = 160;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTTILE32 && chunk < STARTTILE32 + NUMTILE32) {
        expanded = 512;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTTILE32M && chunk < STARTTILE32M + NUMTILE32M) {
        expanded = 640;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    } else if (chunk >= STARTSPRITES && chunk < STARTTILE8) {
        CAL_CacheSprite(chunk, source);
    } else {
        expanded = *(long *)source;
        source += 4;
        MM_GetPtr((memptr *)&grsegs[chunk], expanded);
        if (grsegs[chunk])
            CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
    }
}

void CA_CacheGrChunk(int chunk)
{
    long pos, compressed;
    int next;
    byte *source;
    byte bigbuffer[4096];
    byte *bigbufferseg = NULL;

    grneeded[chunk] |= ca_levelbit;

    if (chunk < 0 || chunk >= NUMCHUNKS)
        return;

    if (grsegs[chunk]) {
        MM_SetPurge(&grsegs[chunk], 0);
        return;
    }

    if (grhandle == -1 || !grstarts)
        return;

    pos = grstarts[chunk];
    if (pos < 0)
        return;

    next = chunk + 1;
    while (next < NUMCHUNKS && grstarts[next] < 0)
        next++;
    if (next >= NUMCHUNKS)
        compressed = 0;
    else
        compressed = grstarts[next] - pos;

    if (compressed <= 0)
        return;

    lseek(grhandle, pos, SEEK_SET);

    if (compressed <= (long)sizeof(bigbuffer)) {
        CA_FarRead(grhandle, bigbuffer, compressed);
        source = bigbuffer;
    } else {
        bigbufferseg = (byte *)malloc(compressed);
        if (!bigbufferseg) return;
        CA_FarRead(grhandle, bigbufferseg, compressed);
        source = bigbufferseg;
    }

    CAL_ExpandGrChunk(chunk, source);

    if (bigbufferseg)
        free(bigbufferseg);
}

void CA_CacheMap(int mapnum)
{
    long pos, compressed;
    unsigned expanded;
    unsigned long size;
    int plane;
    unsigned char *hp;
    long *offsets;
    maptype *maphdr;
    unsigned rlewtag;
    unsigned *buf;
    int i;

    if (mapnum < 0 || mapnum >= NUMMAPS)
        return;

    if (mapon >= 0) {
        for (i = 0; i < 3; i++)
            if (mapsegs[i])
                MM_FreePtr((memptr *)&mapsegs[i]);
    }

    if (maphandle == -1 || !tinf)
        return;

    rlewtag = *(unsigned *)tinf;
    offsets = (long *)(tinf + 2);
    hp = (unsigned char *)(tinf + 2 + 400);

    pos = offsets[mapnum];
    if (pos < 0)
        Quit("Tried to load a non existant map!");

    if (!mapheaderseg[mapnum])
        MM_GetPtr((memptr *)&mapheaderseg[mapnum], sizeof(maptype));
    if (!mapheaderseg[mapnum]) return;
    maphdr = mapheaderseg[mapnum];

    compressed = hp[mapnum];
    if (compressed <= 0)
        compressed = 256;

    lseek(maphandle, pos, SEEK_SET);

    if (compressed < 4096) {
        CA_FarRead(maphandle, bufferseg, compressed);
        CAL_HuffExpand(bufferseg, (byte *)maphdr, sizeof(maptype), maphuffman);
    }

    size = maphdr->width * maphdr->height * 2;

    for (plane = 0; plane < 3; plane++) {
        byte *source;
        byte bigbuffer[4096];
        byte *bigbufferseg = NULL;

        MM_GetPtr((memptr *)&mapsegs[plane], size);
        if (!mapsegs[plane]) continue;

        pos = maphdr->planestart[plane];
        compressed = maphdr->planelength[plane];

        lseek(maphandle, pos, SEEK_SET);

        if (compressed <= (long)sizeof(bigbuffer)) {
            CA_FarRead(maphandle, bigbuffer, compressed);
            source = bigbuffer;
        } else {
            bigbufferseg = (byte *)malloc(compressed);
            if (!bigbufferseg) {
                MM_FreePtr((memptr *)&mapsegs[plane]);
                continue;
            }
            CA_FarRead(maphandle, bigbufferseg, compressed);
            source = bigbufferseg;
        }

        expanded = *(unsigned *)source;
        buf = (unsigned *)malloc(expanded);
        if (buf) {
            CAL_HuffExpand(source + 2, (byte *)buf, expanded, maphuffman);
            CA_RLEWexpand(buf + 1, mapsegs[plane], size, rlewtag);
            free(buf);
        }

        if (bigbufferseg)
            free(bigbufferseg);
    }

    mapon = mapnum;
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
    unsigned i;
    for (i = 0; i < NUMSOUNDS; i++)
        CA_CacheAudioChunk(STARTDIGISOUNDS + i);
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
    int i;
    for (i = 0; i < NUMCHUNKS; i++) {
        if (grneeded[i] & ca_levelbit) {
            CA_CacheGrChunk(i);
            if (cachedownlevel)
                grneeded[i] &= ~ca_levelbit;
        }
    }
}
