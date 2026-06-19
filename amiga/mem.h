#ifndef __AMIGA_MEM_WRAPPER__
#define __AMIGA_MEM_WRAPPER__

#include <string.h>
#include <stdlib.h>

/* Borland far-memory compat for flat AmigaOS memory model */
#ifndef _fmemcpy
#define _fmemcpy(d,s,n) memcpy((d),(s),(n))
#endif

#ifndef movedata
#define movedata(sseg,soff,dseg,doff,n) memcpy((void*)(doff),(void*)(soff),(n))
#endif

#endif
