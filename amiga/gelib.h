#ifndef __GELIB_H__
#define __GELIB_H__

#include "sl_file.h"
#include "id_mm.h"

struct BitMapHeader {
    unsigned int w, h, x, y;
    unsigned char d, trans, comp, pad;
};

struct BitMap {
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int BytesPerRow;
    char *Planes[8];
};

struct Shape {
    memptr Data;
    long size;
    unsigned int BPR;
    struct BitMapHeader bmHdr;
};

void FreeShape(struct Shape *shape);
int UnpackEGAShapeToScreen(struct Shape *SHP, int startx, int starty);
long Verify(char *filename);
memptr InitBufferedIO(int handle, BufferedIO *bio);
void FreeBufferedIO(BufferedIO *bio);
byte bio_readch(BufferedIO *bio);
void bio_fillbuffer(BufferedIO *bio);
void SwapLong(long far *Var);
void SwapWord(unsigned int far *Var);
void MoveGfxDst(short x, short y);

#endif
