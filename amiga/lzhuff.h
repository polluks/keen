#ifndef __LZHUFF_H__
#define __LZHUFF_H__

extern void (*LZH_CompressDisplayVector)();
extern void (*LZH_DecompressDisplayVector)();

long lzhCompress(void *infile, void *outfile, unsigned long DataLength, unsigned PtrTypes);
long lzhDecompress(void *infile, void *outfile, unsigned long OrginalLength, unsigned long CompressLength, unsigned PtrTypes);

#endif
