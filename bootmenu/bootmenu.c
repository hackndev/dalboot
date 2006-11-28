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
	pxa_gpio_mode(GPIO_NR_PALMLD_GREEN_LED_MD);
	SET_PALMLD_GPIO(GREEN_LED, 1);

	init_video();
	make_it_pink();
	while (1);
}
