#if 0
#include "pxa-regs.h"
#include "bootmenu.h"

extern const unsigned char fontdata_8x16[];

#define RGB16(r,g,b) ((r<<11)+(g<<5)+b)
#define FONT_ROWS 16
#define FONT_COLS 8

static u16 *framebuffer;
static u32 width, height, bpp;
static u32 cursorx, cursory;

void init_video()
{
	framebuffer = *(u16**)(FDADR0 + 4);
	width = (LCCR1 & 0x3ff) + 1;
	height = (LCCR2 & 0x3ff) + 1;
	bpp = 16;
}

#endif
