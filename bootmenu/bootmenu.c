#include "palmld-gpio.h"
#include "bootmenu.h"

/**
 * slapin's beautiful malloc implementation.
 */
static void * sbrk = NULL;

void * cheap_malloc(int bytes)
{
//	print ( "cheapo_malloc(): " );
//	printf("sbrk=%lx->",(u32)sbrk);
//	if ( !sbrk ) sbrk = (void *)&_end;
//	printf ("%lx->",(u32)sbrk);
	void * val = sbrk;
	int tmp;
	for(tmp=0;tmp<bytes;tmp++)
	{
//		printf("%lx %x\n",(u32)sbrk,tmp);
//		print(".");
		*((int*)(sbrk+tmp)) = 0;
	}
	sbrk+=bytes;
//	printf("%lx\n",(u32)sbrk);
	return val;
}

/*
 * Handy function to set GPIO alternate functions
 *  From:	linux/arch/arm/mach-pxa/generic.c
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 */

void pxa_gpio_mode(int gpio_mode)
{
	int gpio = gpio_mode & GPIO_MD_MASK_NR;
	int fn = (gpio_mode & GPIO_MD_MASK_FN) >> 8;
	int gafr;

	if (gpio_mode & GPIO_DFLT_LOW)
		GPCR(gpio) = GPIO_bit(gpio);
	else if (gpio_mode & GPIO_DFLT_HIGH)
		GPSR(gpio) = GPIO_bit(gpio);
	if (gpio_mode & GPIO_MD_MASK_DIR)
		GPDR(gpio) |= GPIO_bit(gpio);
	else
		GPDR(gpio) &= ~GPIO_bit(gpio);
	gafr = GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2));
	GAFR(gpio) = gafr |  (fn  << (((gpio) & 0xf)*2));
}

extern u32 _start;
extern u32 _end;
extern u32 _length;

int main()
{
//	u32 machcode[2] = {0,0};
	pxa_gpio_mode(GPIO_NR_PALMLD_GREEN_LED_MD);
	SET_PALMLD_GPIO(GREEN_LED, 1);

	init_video();
	fill_screen(RGB16(5,5,5));
	puts("Bootmenu v0.2\nBy Fahrzin Hemmati\n");

	u32 cpu = get_cpu();
	printf("[CPU] %s %s\n", get_cpu_vendor(cpu), get_cpu_name(cpu));
	
	printf("_start: %lx\n_end: %lx\nsbrk: %lx\n",(u32)&_start,(u32)&_end,(u32)sbrk);
	sbrk = (void *)&_end;
	printf("sbrk: %lx\n",(u32)sbrk);

	//machcode[0] = get_rom_mach();
	//printf("[MACH] %s\n", (char*)machcode);

	init_palmld();
	init_palmcard();
	
//	read_a_file("/linux.txt");

	test_fat();
	
	
	int k;
	while (1) {
		k = getchar();
		if (k) putchar(k);
		switch (k) {
		case 'h': return 0; /* home: boot palmos */
		}
	}
	return 0;
}

/**
 * Copy bootmenu from wherever it currently is to the
 * _start location defined by the linker.
 *
 * start.S will pass in the current address we're located
 * at and expects the final address of the main function
 * in return.
 */
void *relocate_bootmenu(u32 *src)
{
	u32 *dest=&_start;
	while (dest < &_end)
		*(dest++) = *(src++);
	return &main;
}

/**
 * Needed by libgcc for div by zero errors etc.
 */
int raise(int sig)
{
	puts("libgcc error. div by zero?\n");
	while(1);
}

/**
 * fahhem's first attempt at a malloc AND free
 *
 * If an empty byte is put between variables, free doesn't have to take a sizeof parameter
 */
/*
static u32 mems[100]; //each bit refers to 8bits of memory
static u32 * memend = mems+sizeof(mems);

void * fahhem_malloc(int bytes)
{
	if (!bytes) return 0;
	
	u32 * cm=mems;
	u32 size = 0;
	u32 x;
	for(x=0;x<(bytes&0x1f);x++) size |= 1<<x;
	while(cm<memend)
	{
		if(bytes>32 && *cm) continue;
		for(x=0;x<32-(bytes&0x1f);x++)
			if(!(*cm & (size<<x)))
			{
				*cm|=size<<x; //set to used
				u32 add = ((cm-mems)<<4) + x;
				return ((void *)&_end)+add;
			}
		cm++;

	}
	return 0;
}

void fahhem_free(void * ptr, int bytes)
{
	if ( ptr < (void *)&_end ) return;
	u32 offset=ptr-((void *)&_end);
	u32 bit = offset & (0x1f);
	u32 mem = offset >> 4;

	u32 size = 0;
        u32 x;
        for(x=0;x<bytes;x++) size |= 1<<x;

	mems[mem] ^= size<<bit;
}
*/

/*
	Sets the LED to red while reading from the drive.
*/
void readsw(const void *a, void *b, int c)
{
	SET_PALMLD_GPIO(GREEN_LED, 0);
	SET_PALMLD_GPIO(ORANGE_LED, 1);
	_readsw(a,b,c);
	SET_PALMLD_GPIO(ORANGE_LED, 0);
//	SET_PALMLD_GPIO(GREEN_LED, 1);
}
