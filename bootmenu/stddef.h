#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned long size_t;
typedef unsigned char u8;
typedef __WCHAR_TYPE__ wchar_t;

#define NULL ((void *)0)

enum {
	false	= 0,
	true	= 1
};

#endif
