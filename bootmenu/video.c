#include "pxa-regs.h"
#include "bootmenu.h"

extern const unsigned char fontdata_8x16[];
unsigned char font_entry(u32 entry);

#define PIXEL(x,y) framebuffer[(x)+width*(y)]
#define FONT_ROWS 16
#define FONT_COLS 8

static u16 *framebuffer;
static u32 width, height, bpp;
static u32 cursorx, cursory;

void init_video()
{
	cursorx=cursory=0;
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

int putc(const char c)
{
	switch (c) {
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
		draw_char(c, cursorx*FONT_COLS, cursory*FONT_ROWS);
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
	return 1;
}

int puts(const char *s)
{
	while(*s) {
		putc(*s);	
		s++;
	}
	return 1;
}

void put_pc()
{
	u32 pc;
	asm("test:mov %0, pc" : "=r"(pc));
	while (pc) {
		putc('0' + (pc%10));
		pc /= 10;
	}
}

void fill_screen(u16 color)
{
	u32 len = width * height * bpp / 8 /2;
	u32 i;
	if (!framebuffer) return;
	
	for (i=0; i < len; i++)
		framebuffer[i] = color;

	cursorx = cursory = 0;
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
	put_pc();
	putc('\n');
	putc('A'); putc('l'); putc('e'); putc('x');
	putc('\n'); putc('H'); putc('&'); putc('D');
	puts("\nHello");
}