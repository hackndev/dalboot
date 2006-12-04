#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned long size_t;
typedef unsigned char u8;

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

enum {
	false	= 0,
	true	= 1
};

#endif
