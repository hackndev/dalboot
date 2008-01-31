#ifndef _BOOTMENU_H_
#define _BOOTMENU_H_

#include "stddef.h"
#include "fs/drive.h"

#define RAM_BASE 0xa0000000

/* video.c */
void fill_screen(u16 color);
void init_video();
void printf_ch_bin(u8 bin);
int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);
#define print(x) puts(x)
//int inline print(const char *txt);
void switch_led();
void disp_clear();
#define RGB16(r,g,b) ((r<<11)+(g<<5)+b)

/* start.S */
u32 get_cpu_id(void);

/* mach.c */
void init_mach();
int cpu_is_pxa();
u32 get_cpu();
u32 get_rom_mach();
const char *get_cpu_vendor(u32 cpu);
const char *get_cpu_name(u32 cpu);

/* bootmenu.c */
void pxa_gpio_mode(int gpio_mode);
void *cheap_malloc(int bytes);
//void *fahhem_malloc(int bytes);
//void fahhem_free(void * ptr);
#define malloc(x) cheap_malloc(x)
#define free(x) do{ }while(0)
//#define malloc(x) fahhem_malloc(x)
//#define free(x,y) fahhem_free(x,y)


/* keypad.c */
typedef struct keypad_matrix {
	int rows, cols;
	int pxa27x; /* use the pxa27x keypad controller? */
	int in_gpios[8], out_gpios[8];
	int keymap[8][8];
} keypad_matrix;
void init_keypad(keypad_matrix *keypad);
int read_keypad();
int getchar();
void wait_input();

/* palmcard.c */
void init_palmcard();

/* drive.c */
//void init_drive();
//void test_drive();
//int fdc_fdos_read(void *buffer, int len);
//int fdc_fdos_seek(int where);
//void read_a_file(char * name);

/* io-readsw-armv4.S */
void _readsw(const void *addr, void *data, int wordlen);
void readsw(const void *a, void *b, int c);

/* machine codes (reversed) */
#define PALMT3 'aAz1'
#define PALMT5 'ANGS'
//#define PALMLD 'BRMA'
//#define PALMLD (('B'<<24)+('R'<<16)+('M'<<8)+('A'<<0))
#define PALMLD 0x42524D41

/* machine init */
void init_palmld();

#endif
