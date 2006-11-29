CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -fpic -Wall -O2 -pipe

SRCS=start.S bootmenu.c font.S video.c mach.c

all: BigDAL-pm.prc

BigDAL-pm.prc: bootmenu.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu.bin

bootmenu.bin: bootmenu.elf
	${OBJCOPY} -O binary bootmenu.elf bootmenu.bin

bootmenu.elf: ${SRCS}
	${CC} -S -fverbose-asm $(LDFLAGS) ${SRCS} > /dev/null
	${CC} $(LDFLAGS) ${SRCS} -o bootmenu.elf

clean:
	rm -f bootmenu.bin bootmenu.elf BigDAL-pm.prc *.s
