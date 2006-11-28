CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -fPIC

SRCS=start.S bootmenu.c

all: BigDAL-pm.prc

BigDAL-pm.prc: bootmenu.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu.bin

bootmenu.bin: bootmenu.elf
	${OBJCOPY} -O binary bootmenu.elf bootmenu.bin

bootmenu.elf: ${SRCS}
	${CC} $(LDFLAGS) ${SRCS} -o bootmenu.elf

clean:
	rm -f bootmenu.bin bootmenu.elf BigDAL-pm.prc
