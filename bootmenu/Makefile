CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -Wall -pipe -Tbootmenu.lds

SRCS=start.S bootmenu.c font.S video.c mach.c vsprintf.c string.c ctype.c \
	div64.S keypad.c
LIBGCC=-lgcc

all: BigDAL-pm.prc

BigDAL-pm.prc: bootmenu.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu.bin

bootmenu.bin: bootmenu.elf
	${OBJCOPY} -O binary bootmenu.elf bootmenu.bin

bootmenu.elf: ${SRCS} bootmenu.lds
	${CC} -S -fverbose-asm $(LDFLAGS) ${SRCS} > /dev/null
	${CC} $(LDFLAGS) ${SRCS} ${LIBGCC} -o bootmenu.elf

clean:
	rm -f bootmenu.bin bootmenu.elf BigDAL-pm.prc *.s
