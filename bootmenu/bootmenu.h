
#include "stddef.h"

#define RAM_BASE 0xa0000000

/* video.c */
void fill_screen(u16 color);
void init_video();
int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);
int inline print(const char *txt);
void switch_led();
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
#define malloc(x) cheap_malloc(x)
#define free(x) do { } while(0)


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

/* palmcard.c */
void init_palmcard();

/* ide.c */
void init_ide();
int fdc_fdos_read(void *buffer, int len);
int fdc_fdos_seek(int where);

/* io-readsw-armv4.S */
void readsw(const void *addr, void *data, int wordlen);

/* machine codes (reversed) */
#define PALMT3 'aAz1
#define PALMT5 'ANGS'
#define PALMLD 'BRMA'

/* machine init */
void init_palmld();
