#ifndef __AMIGA_STRING_WRAPPER__
#define __AMIGA_STRING_WRAPPER__

#include <string.h>

/* Borland extensions not in standard C */
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

static __inline char *_strlwr(char *s)
{
    char *p = s;
    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }
    return s;
}

#endif
