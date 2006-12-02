
#include "stddef.h"
/* video.c */
void fill_screen(u16 color);
void init_video();
int putc(const char c);
int puts(const char *s);
int printf(const char *fmt, ...);
#define RGB16(r,g,b) ((r<<11)+(g<<5)+b)

/* start.S */
u32 get_cpu_id(void);

/* mach.c */
int cpu_is_pxa();
u32 get_cpu();
const char *get_cpu_vendor(u32 cpu);
const char *get_cpu_name(u32 cpu);
