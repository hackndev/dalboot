#include "pxa-regs.h"
#include "bootmenu.h"

extern const unsigned char fontdata_8x16[];

#define RGB16(r,g,b) ((r<<11)+(g<<5)+b)

static u16 *framebuffer;
static u32 width, height, bpp;

void init_video()
{
	framebuffer = *(u16**)(FDADR0 + 4);
	width = (LCCR1 & 0x3ff) + 1;
	height = (LCCR2 & 0x3ff) + 1;
	bpp = 16;
}

void make_it_pink()
{
	u32 len = width * height * bpp / 8/4;
	u16 *p;
	
	for (p=framebuffer; p < framebuffer+len; p++) {
		*p = RGB16(5,5,15);
	}
}


