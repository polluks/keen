#ifndef _SL_FILE_H
#define _SL_FILE_H

#ifndef MakeID
#define MakeID(a,b,c,d) (((long)(d)<<24L)|((long)(c)<<16L)|((long)(b)<<8L)|(long)(a))
#endif

#define ID_SLIB   MakeID('S','L','I','B')
#define SLIB      ("SLIB")
#define SOFTLIB_VER 2
#define ID_CHUNK  MakeID('C','U','N','K')

typedef enum LibFileTypes {
    lib_DATA = 0
} LibFileTypes;

typedef struct SoftLibHdr {
    unsigned Version;
    unsigned FileCount;
} SoftlibHdr;

#define SL_FILENAMESIZE 16

typedef struct FileEntryHdr {
    char FileName[SL_FILENAMESIZE];
    unsigned long Offset;
    unsigned long ChunkLen;
    unsigned long OrginalLength;
    short Compression;
} FileEntryHdr;

typedef struct ChunkHeader {
    unsigned long HeaderID;
    unsigned long OrginalLength;
    short Compression;
} ChunkHeader;

#endif
