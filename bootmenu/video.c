#include <stdarg.h>
#include "pxa-regs.h"
#include "palmld-gpio.h"
#include "bootmenu.h"
#include "vsprintf.h"

extern const unsigned char fontdata_8x16[];
unsigned char font_entry(u32 entry);

#define PIXEL(x,y) framebuffer[(x)+width*(y)]
#define FONT_ROWS 16
#define FONT_COLS 8

#define SWITCHCALLS 127

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

void printf_ch_bin(u8 bin)
{
	u8 n;
	for(n=0; n<8; n++)
	{
		if(bin & 0x1)	print("1");
		else		print("0");
		if (n==3)	print(" "); // groups of 4

		bin >>= 1;
	}
}


int putchar(int c)
{
	switch (c) {
	case '\0':
		return 1;
	case '\n':
		cursory++;
//		if(cursory>=height/FONT_ROWS) { wait_input(); disp_clear(); }
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
	if (cursorx*FONT_COLS >= width) {
		cursorx=0;
		cursory++;
	}
	if (cursory*FONT_ROWS >= height) {
		wait_input();
		disp_clear();
		cursory=0;
	}
	return 1;
}

int puts(const char *s)
{
	while(*s) {
		putchar(*s);	
		s++;
	}
	return 1;
}

int printf(const char *fmt, ...)
{
	char buf[256];
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsnprintf(buf,256,fmt,args);
	va_end(args);
	puts(buf);
	return i;
}

/*
int inline print(const char *txt)
{
	return puts(txt);
}
*/

void put_pc()
{
	u32 pc;
	asm("test:mov %0, pc" : "=r"(pc));
	while (pc) {
		putchar('0' + (pc%10));
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

	put_pc();
}

static u16 tillswitch = 0;

void switch_led()
{
	if (tillswitch==SWITCHCALLS) {
		if(GET_PALMLD_GPIO(GREEN_LED))
			SET_PALMLD_GPIO(GREEN_LED, 0);
		else
			SET_PALMLD_GPIO(GREEN_LED, 1);
		tillswitch++;
	}else{
		tillswitch++;
	}
}

void disp_clear()
{
	fill_screen(RGB16(5,5,5));
}
