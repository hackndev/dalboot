#define NULL ((void*)0)
typedef unsigned long u32;
typedef unsigned short u16;

/* video.c */
void make_it_pink();
void init_video();

/* start.S */
u32 get_cpu_id(void);

/* mach.c */
int cpu_is_pxa();
