#ifndef __ID_GLOB__
#define __ID_GLOB__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "conio.h"

#include <exec/types.h>
#include <exec/memory.h>

#define __ID_GLOB__

#define EXTENSION "KDR"

#include "graphkdr.h"
#include "audiokdr.h"

#define TEXTGR 0
#define CGAGR  1
#define EGAGR  2
#define VGAGR  3

#define GRMODE EGAGR

#define boolean int
#define true    1
#define false   0

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   longword;
typedef byte *          Ptr;
typedef void *          memptr;

typedef struct { int x, y; } Point;
typedef struct { Point ul, lr; } Rect;

#define nil ((void *)0)

#define _seg
#define far
#define huge
#define interrupt

#ifndef FP_SEG
#define FP_SEG(p) 0
#endif
#ifndef FP_OFF
#define FP_OFF(p) ((unsigned)(p))
#endif

extern void Quit(char *error);

#ifndef stricmp
#define stricmp strcasecmp
#endif

#include "id_mm.h"
#include "id_ca.h"
#include "id_vw.h"
#include "id_rf.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"

#endif
