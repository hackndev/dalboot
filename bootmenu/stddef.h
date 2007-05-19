#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define NULL ((void *)0)

enum {
	false	= 0,
	true	= 1
};


typedef unsigned long size_t;

#endif
