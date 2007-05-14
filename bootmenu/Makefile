CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -Wall -pipe -Tbootmenu.lds

SRCS=start.S bootmenu.c font.S video.c mach.c vsprintf.c string.c ctype.c \
	div64.S keypad.c palmcard.c palmld.c io-readsw-armv4.S \
	fs/drive.c  fs/fat.c
#fs/ide.c fs/dev.c fs/fat.c fs/fdos.c fs/fs.c fs/subdir.c fs/vfat.c

LIBGCC=-lgcc

all: BigDAL-bm.prc zImage

zImage: bootmenu.bin
	cp bootmenu.bin zImage

BigDAL-bm.prc: bootmenu.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu.bin

bootmenu.bin: bootmenu.elf
	${OBJCOPY} -O binary bootmenu.elf bootmenu.bin

bootmenu.elf: ${SRCS} bootmenu.lds bootmenu.h
	${CC} -S -fverbose-asm $(LDFLAGS) ${SRCS} > /dev/null
	${CC} $(LDFLAGS) ${SRCS} ${LIBGCC} -o bootmenu.elf

clean:
	rm -f bootmenu.bin bootmenu.elf BigDAL-bm.prc *.s
