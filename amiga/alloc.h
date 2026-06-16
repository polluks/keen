#ifndef __AMIGA_ALLOC_WRAPPER__
#define __AMIGA_ALLOC_WRAPPER__

#include <stdlib.h>

#define farmalloc(n) malloc(n)
#define farfree(p) free(p)

#endif
