CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
LD=${CROSS_COMPILE}ld
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -Wall -pipe -Tbootmenu.lds

SRCS=start.S main.c font.S video.c mach.c vsprintf.c string.c ctype.c \
	div64.S keypad.c palmcard.c palmld.c io-readsw-armv4.S \
	fs/drive.c fs/fat.c fs/cache.c config.c fs/fakestdio.c \
	bootmenu.c

LIBGCC=-lgcc

ASMS=start.S main.s font.S video.s mach.s vsprintf.s string.s ctype.s \
	div64.S keypad.s palmcard.s palmld.s io-readsw-armv4.S \
	drive.s fat.s cache.s config.s fakestdio.s \
	main.s

ASMLIBGCC=/usr/lib/gcc/arm-softfloat-linux-gnu/4.1.1/libgcc.a


all: BigDAL-bm.prc zImage

zImage: bootmenu.bin
	cp bootmenu.bin zImage

BigDAL-bm.prc: bootmenu.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu.bin

bootmenu.bin: bootmenu.elf
	${OBJCOPY} -O binary bootmenu.elf bootmenu.bin

bootmenu.elf: ${SRCS} bootmenu.lds bootmenu.h
	${CC} $(LDFLAGS) ${SRCS} ${LIBGCC} -o bootmenu.elf

debug: ${SRCS}
	${CC} -S -fverbose-asm $(LDFLAGS) ${SRCS} > /dev/null

clean:
	rm -f bootmenu.bin bootmenu.elf BigDAL-bm.prc *.s



asm: debug BigDAL-bm-asm.prc zImage-asm
	mv BigDAL-bm-asm.prc BigDAL-bm.prc

zImage-asm: bootmenu-asm.bin
	cp bootmenu-asm.bin zImage-asm

BigDAL-bm-asm.prc: bootmenu-asm.bin
	python ../dalmod/dalmod.py ../BigDAL.prc bootmenu-asm.bin
	mv BigDAL-bm.prc BigDAL-bm-asm.prc

bootmenu-asm.bin: bootmenu-asm.elf
	${OBJCOPY} -O binary bootmenu-asm.elf bootmenu-asm.bin

bootmenu-asm.elf: ${ASMS} bootmenu.lds bootmenu.h
	${CC} ${LDFLAGS} ${LIBGCC} ${ASMS} ${ASMLIBGCC} -o bootmenu-asm.elf

