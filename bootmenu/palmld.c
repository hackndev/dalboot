#include "bootmenu.h"
#include "palmld-gpio.h"

static keypad_matrix palmld_keypad = {
	.rows = 4,
	.cols = 3,
	.pxa27x = true,
	.in_gpios = {
		GPIO_NR_PALMLD_KP_MKIN0_MD,
		GPIO_NR_PALMLD_KP_MKIN1_MD,
		GPIO_NR_PALMLD_KP_MKIN2_MD,
		GPIO_NR_PALMLD_KP_MKIN3_MD,
		-1,
	},
	.out_gpios = {
		GPIO_NR_PALMLD_KP_MKOUT0_MD,
		GPIO_NR_PALMLD_KP_MKOUT1_MD,
		GPIO_NR_PALMLD_KP_MKOUT2_MD,
		-1,
	},
	.keymap = {
		{
			-1,
			'f',	/* folder */
			'u',	/* up */
		},
		{
			'p',	/* picture */
			's',	/* star */
			'r',	/* right */
		},
		{
			'h',	/* home */
			'v',	/* voice memo */
			'd',	/* down */
		},
		{
			'r',	/* rotate */
			'c',	/* centre */
			'l',	/* left */
		},
	}
};

void init_palmld()
{
	print("Initting LifeDrive\n");
	init_keypad(&palmld_keypad);
//	init_ide();
	print("LifeDrive\n");
}
