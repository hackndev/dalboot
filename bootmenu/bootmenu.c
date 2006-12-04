#include "palmld-gpio.h"
#include "bootmenu.h"

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

int main()
{
	u32 machcode[2] = {0,0};
	pxa_gpio_mode(GPIO_NR_PALMLD_GREEN_LED_MD);
	SET_PALMLD_GPIO(GREEN_LED, 1);

	init_video();
	fill_screen(RGB16(5,5,5));
	puts("Bootmenu v0.1\nBy Alex Osborne\n\n");

	u32 cpu = get_cpu();
	printf("[CPU] %s %s\n", get_cpu_vendor(cpu), get_cpu_name(cpu));

	//machcode[0] = get_rom_mach();
	//printf("[MACH] %s\n", (char*)machcode);

	init_palmld();
	init_palmcard();
	
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
 * Needed by libgcc for div by zero errors etc.
 */
int raise(int sig)
{
	puts("libgcc error. div by zero?\n");
	while(1);
}

/**
 * Copy bootmenu from wherever it currently is to the
 * _start location defined by the linker.
 *
 * start.S will pass in the current address we're located
 * at and expects the final address of the main function
 * in return.
 */
extern u32 _start;
extern u32 _end;
void *relocate_bootmenu(u32 *src)
{
	u32 *dest=&_start;
	while (dest < &_end)
		*(dest++) = *(src++);
	return &main;
}
