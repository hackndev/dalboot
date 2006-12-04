/*
 * include/asm-arm/arch-pxa/palmld-gpio.h
 *
 * Authors: Alex Osborne <bobofdoom@gmail.com>
 *
 */

#ifndef _PALMLD_GPIO_H_
#define _PALMLD_GPIO_H_

#include "pxa-regs.h"

/* IO mappings */
#define PALMLD_USB_PHYS		(PXA_CS2_PHYS)
#define PALMLD_USB_VIRT		(0xf0000000)
#define PALMLD_USB_SIZE		(0x00100000)

#define PALMLD_IDE_PHYS		(0x20000000)
#define PALMLD_IDE_VIRT		(0xf1000000)
#define PALMLD_IDE_SIZE		(0x00100000)


/* Palm LifeDrive GPIOs */
#define GPIO_NR_PALMLD_USB_DETECT		3
#define GPIO_NR_PALMLD_POWER_DETECT		4
#define GPIO_NR_PALMLD_HOTSYNC_BUTTON_N		10
#define GPIO_NR_PALMLD_POWER_SWITCH		11
#define GPIO_NR_PALMLD_EARPHONE_DETECT 		13
#define GPIO_NR_PALMLD_SD_DETECT_N		14	/* SD card inserted */
#define GPIO_NR_PALMLD_LOCK_SWITCH		15	/* keypad lock */

#define GPIO_NR_PALMLD_WM9712_IRQ		27

#define GPIO_NR_PALMLD_STD_RXD			46	/* IRDA */
#define GPIO_NR_PALMLD_STD_TXD			47

#define GPIO_NR_PALMLD_GREEN_LED        52

#define GPIO_NR_PALMLD_ORANGE_LED	94
#define GPIO_NR_PALMLD_IDE_IRQ		95

#define GPIO_NR_PALMLD_KP_MKIN3		97		/* rotate-display, center, left */
#define GPIO_NR_PALMLD_IDE_RESET	98

#define GPIO_NR_PALMLD_KP_MKIN0		100 	/* folder, up */
#define GPIO_NR_PALMLD_KP_MKIN1		101 	/* picture, star, right */
#define GPIO_NR_PALMLD_KP_MKIN2		102 	/* voice memo, home, down */

#define GPIO_NR_PALMLD_KP_MKOUT0	103
#define GPIO_NR_PALMLD_KP_MKOUT1	104
#define GPIO_NR_PALMLD_KP_MKOUT2	105

#define GPIO_NR_PALMLD_IRDA_SD		108	/* IRDA shutdown, active high */

#define GPIO_NR_PALMLD_IDE_PWEN		115

#define GPIO_NR_PALMLD_GREEN_LED_MD	(GPIO_NR_PALMLD_GREEN_LED | GPIO_OUT)

#define GPIO_NR_PALMLD_STD_RXD_MD		(GPIO_NR_PALMLD_STD_RXD | GPIO_ALT_FN_2_IN)
#define GPIO_NR_PALMLD_STD_TXD_MD		(GPIO_NR_PALMLD_STD_TXD | GPIO_ALT_FN_1_OUT)
#define GPIO_NR_PALMLD_IDE_PWEN_MD		(GPIO_NR_PALMLD_IDE_PWEN | GPIO_OUT)

#define GPIO_NR_PALMLD_KP_MKIN3_MD 	(GPIO_NR_PALMLD_KP_MKIN3 | GPIO_ALT_FN_3_IN)
#define GPIO_NR_PALMLD_KP_MKIN0_MD 	(GPIO_NR_PALMLD_KP_MKIN0 | GPIO_ALT_FN_1_IN)
#define GPIO_NR_PALMLD_KP_MKIN1_MD 	(GPIO_NR_PALMLD_KP_MKIN1 | GPIO_ALT_FN_1_IN)
#define GPIO_NR_PALMLD_KP_MKIN2_MD 	(GPIO_NR_PALMLD_KP_MKIN2 | GPIO_ALT_FN_1_IN)

#define GPIO_NR_PALMLD_KP_MKOUT0_MD (GPIO_NR_PALMLD_KP_MKOUT0 | GPIO_ALT_FN_2_OUT)
#define GPIO_NR_PALMLD_KP_MKOUT1_MD (GPIO_NR_PALMLD_KP_MKOUT1 | GPIO_ALT_FN_2_OUT)
#define GPIO_NR_PALMLD_KP_MKOUT2_MD (GPIO_NR_PALMLD_KP_MKOUT2 | GPIO_ALT_FN_2_OUT)

#define IRQ_GPIO_PALMLD_SD_DETECT_N	IRQ_GPIO(GPIO_NR_PALMLD_SD_DETECT_N)
#define IRQ_GPIO_PALMLD_WM9712_IRQ	IRQ_GPIO(GPIO_NR_PALMLD_WM9712_IRQ)
#define IRQ_GPIO_PALMLD_IDE_IRQ		IRQ_GPIO(GPIO_NR_PALMLD_IDE_IRQ)

/* Utility macros */
#define GET_PALMLD_GPIO(gpio) \
        (GPLR(GPIO_NR_PALMLD_ ## gpio) & GPIO_bit(GPIO_NR_PALMLD_ ## gpio))

#define SET_PALMLD_GPIO(gpio, setp) \
do { \
if (setp) \
        GPSR(GPIO_NR_PALMLD_ ## gpio) = GPIO_bit(GPIO_NR_PALMLD_ ## gpio); \
else \
        GPCR(GPIO_NR_PALMLD_ ## gpio) = GPIO_bit(GPIO_NR_PALMLD_ ## gpio); \
} while (0)

#define SET_PALMLD_GPIO_N(gpio, setp) \
do { \
if (setp) \
        GPCR(GPIO_NR_PALMLD_ ## gpio) = GPIO_bit(GPIO_NR_PALMLD_ ## gpio); \
else \
        GPSR(GPIO_NR_PALMLD_ ## gpio) = GPIO_bit(GPIO_NR_PALMLD_ ## gpio); \
} while (0)


#define GET_GPIO(gpio) (GPLR(gpio) & GPIO_bit(gpio))


#endif /* _PALMLD_GPIO_H_ */
