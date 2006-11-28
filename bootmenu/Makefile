CROSS_COMPILE?=arm-softfloat-linux-gnu-
CC=${CROSS_COMPILE}gcc
LDFLAGS=-nostdlib -fPIC

SRCS=start.S bootmenu.c

all:
	${CC} $(LDFLAGS) ${SRCS} -o bootmenu.elf
