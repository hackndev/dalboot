#include "pxa-regs.h"
#include "bootmenu.h"

extern const unsigned char fontdata_8x16[];
unsigned char font_entry(u32 entry);

#define PIXEL(x,y) framebuffer[(x)+width*(y)]
#define RGB16(r,g,b) ((r<<11)+(g<<5)+b)
#define FONT_ROWS 16
#define FONT_COLS 8

static u16 *framebuffer;
static u32 width, height, bpp;
static u32 cursorx, cursory;

void init_video()
{
	if (cpu_is_pxa() && FDADR0) {
		framebuffer = *(u16**)(FDADR0 + 4);
		width = (LCCR1 & 0x3ff) + 1;
		height = (LCCR2 & 0x3ff) + 1;
		bpp = 16;
	} else {
		framebuffer = NULL;
	}
}

void draw_char(unsigned int c, u32 x, u32 y)
{
	u32 row, col;
	if (!framebuffer) return;
	
	for (row=0; row < FONT_ROWS; row++) {
	for (col=0; col < FONT_COLS; col++) {
		if (font_entry((FONT_ROWS*c)+row) & (1<<(7-col))) {
			PIXEL(x+col, y+row) = 0xff00;
		} else {
			PIXEL(x+col, y+row) = 0x0;
		}
	}
	}
}

int puts(const char *s)
{
	while(1) {
		switch (*s) {
		case '\0':
			return 1;
		case '\n':
			cursory++;
		case '\r':
			cursorx=0;
			break;
		case '\t':
			cursorx+=8;
			break;
		case '\b':
			cursorx--;
			break;
		default:
			draw_char(*s, cursorx*FONT_COLS, cursory*FONT_ROWS);
			cursorx++;
		}
		
		/* TODO: better overflow handling */
		if (cursorx*FONT_COLS > width) {
			cursorx=0;
			cursory++;
		}
		if (cursory*FONT_ROWS > height) {
			cursory=0;
		}
		s++;
	}
}

void make_it_pink()
{
	u32 len = width * height * bpp / 8/4;
	u16 *p;
	if (!framebuffer) return;
	
	for (p=framebuffer; p < framebuffer+len; p++) {
		if (fontdata_8x16[0] != 0x70) {
			*p =  RGB16(15,0,0); // red screen of death
		} else {
			*p = RGB16(5,5,15);
		}
	}

	//draw_char('A',0,0);
	puts("Hello Hack&Dev\nFrom Alex\n");
}
