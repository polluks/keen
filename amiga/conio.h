#ifndef __AMIGA_CONIO_WRAPPER__
#define __AMIGA_CONIO_WRAPPER__

static __inline void clrscr(void) {}
static __inline int bioskey(int cmd) { (void)cmd; return 0; }

#endif
