#ifndef FAT_FS_H
#define FAT_FS_H

#include "../stddef.h"

#define GCC_OPTION(x) __attribute__ ((x))

typedef struct
{
	u8 name[11];			/* 0  Short name				*/
	u8 attribs;			/* 11 Attributes				*/
	u8 lower_case;			/* 12 Reserved for NT...NOT! Shows case		*/
	u8 crtd_time_tenth;		/* 13 Tenths of seconds, 0-199 for creation	*/
	u16 crtd_time;			/* 14 Time of creation				*/
	u16 crtd_date;			/* 16 Date of creation				*/
	u16 access_date;		/* 18 Date of last access. Read or write	*/
	u16 first_clust_hi;		/* 20 High word of first cluster		*/
	u16 wrtn_time;			/* 22 Time of last write			*/
	u16 wrtn_date;			/* 24 Date of last write			*/
	u16 first_clust_lo;		/* 26 Low word of first cluster			*/
	u32 file_size;			/* 28 File size in bytes			*/
} GCC_OPTION(packed) fat_dir_entry;

typedef struct
{
	fat_dir_entry * entry;		/* A copy of this file's entry			*/
	u8 * path;			/* The path leading up to this file('/'='\0')	*/
	u8 * name;			/* Points to part of path that indicates name	*/
	int offset;			/* Offset (in sectors) into file		*/
	char * base;			/* Pointer to base of buffer			*/
	char * ptr;			/* Pointer to current position in buffer	*/
	int count;			/* Number of bytes left in the buffer		*/
} GCC_OPTION(packed) FILE;

FILE * fat32_open_file(u8 * path);
u32 fat32_read_file(FILE * file, u32 start, u32 length, u8 * buffer);


#endif
