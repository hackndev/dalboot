#ifndef FAT_FAT_H
#define FAT_FAT_H

#include "cache.h"
#include "../bootmenu.h"

#define GCC_OPTION(x) __attribute__ ((x))
#define ENTRY_FIRST_CLUSTER(x) (((u32)(x)->first_clust_hi<<16) + (x)->first_clust_lo)

#define DEBUG
#ifdef DEBUG
#define PRINTF(fmt,args...)	printf (fmt ,##args)
#define PRINT(fmt)		print(fmt)
#else
#define PRINTF(fmt,args...)
#define PRINT(fmt)
#endif

//#define FAT_FUNCS

typedef struct
{
	u8 jump [3];			/* 0  Unconditional Jump to boot code             */
	u8 oem_name [8];		/* 3  OEM name & version                          */
	u16 sector_size;		/* 11 Bytes per sector hopefully 512              */
	u8 cluster_size;		/* 13 Cluster size in sectors, power of 2         */
	u16 reserved_sectors;		/* 14 Number of reserved (boot) sectors           */
	u8 num_fat;			/* 16 Number of FAT tables hopefully 2            */
	u16 unused1;			/* 17 UNUSED IN FAT32: Number of directory slots  */
	u16 unused2;			/* 19 UNUSED IN FAT32: Total sectors on disk      */
	u8 media_descriptor;		/* 21 Media descriptor and first byte of FAT      */
	u16 unused3;			/* 22 UNUSED IN FAT32: Sectors in FAT             */
	u16 secs_per_track;		/* 24 Sectors/track                               */
	u16 heads;			/* 26 Heads                                       */
	u32 hidden_secs;	       	/* 28 number of hidden sectors                    */
	u32 total_num_secs;		/* 32 total number of sectors                     */

/* FAT32 Specific:									  */

	u32 fat_size;			/* 36 sector size of one fat.			  */
	u16 flags;			/* 40 Flags, active FAT and mirroring		  */
	u16 fat_version;		/* 42 Version of FAT32. MSB=major,LSB=minor	  */
	u32 root_cluster;		/* 44 first cluster of root directory		  */
	u16 fs_info;			/* 48 sector of FSINFO struct			  */
	u16 bckp_boot_sect;		/* 50 sector of backup of boot sector		  */
	u8 reserved[12];		/* 52 reserved for future expansion		  */
	u8 drive_num;			/* 64 drive number				  */
	u8 reserved1;			/* 65 used by Windows NT			  */
	u8 boot_sig;			/* 66 Extended boot signature (0x29)		  */
	u32 volume_id;			/* 67 Volume serial number			  */
	u8 volume_lab[11];		/* 71 Volume label				  */
	u8 filesystem_type[8];		/* 82 Filesystem type, should be "FAT32     "	  */
} GCC_OPTION(packed) bootsector;

typedef struct
{
	u32 leading_signature;		/* 0   Leading signature 0x41615252		*/
	u8 reserved1[480];		/* 4   Reserved for future expansion		*/
	u32 struct_signature;		/* 484 Another signature 0x61417272		*/
	u32 free_count;			/* 488 Last known free count			*/
	u32 next_free;			/* 492 Start looking here for free clusters	*/
	u8 reserved2[12];		/* 496 More reserved crap			*/
	u32 trailing_signature;		/* 508 Trailing signature 0xAA550000		*/
} GCC_OPTION(packed) fat_fs_info;


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
	u8 ordinal;			/* 0  Order. Mask of 0x40 indicates last entry	*/
	u8 name1[10];			/* 1  First 5 characters of name		*/
	u8 attribs;			/* 11 Must be 0x2f				*/
	u8 type;			/* 12 Directory entry???			*/
	u8 checksum;			/* 13 Right-shifted checksum			*/
	u8 name2[12];			/* 14 Characters 6-11 of name			*/
	u16 zero;			/* 26 First cluster. Should be 0		*/
	u8 name3[4];			/* 28 Last 2 characters of name			*/
} GCC_OPTION(packed) fat_long_name_entry;

typedef struct
{
	fat_dir_entry * entry;		/* A copy of this file's entry			*/
	u8 * path;			/* The path leading up to this file('/'='\0')	*/
	u8 * name;			/* Points to part of path that indicates name	*/
} GCC_OPTION(packed) FILE;

#endif
