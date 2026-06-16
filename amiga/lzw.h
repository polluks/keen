#ifndef __LZW_H__
#define __LZW_H__

extern void (*LZW_CompressDisplayVector)();
extern void (*LZW_DecompressDisplayVector)();

unsigned long lzwCompress(void *infile, void *outfile, unsigned long DataLength, unsigned PtrTypes);
void lzwDecompress(void *infile, void *outfile, unsigned long DataLength, unsigned PtrTypes);

#endif
