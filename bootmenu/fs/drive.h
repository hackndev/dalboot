#ifndef FAT_DRIVE_H
#define FAT_DRIVE_H

#include "../pxa-regs.h"
#include "../palmld-gpio.h"
#include "../stddef.h"

void init_drive();
void test_drive();
void test_fat();
//void read_sectors(u32 start, int count, void *buf);
void read(u32 start, int count, void * buf);
void read_sector(u32 start, void * buf);

/* fat.c */
void view_mbr();
void view_bootsector();
void test_fs_driver();

#define SECTOR_SIZE 512

/* IDE registers */
#define IDE_DATA        0
#define IDE_ERRPR       1       /* W: Features */
#define IDE_NSECTOR     2
#define IDE_SECTOR      3
#define IDE_LCYL        4
#define IDE_HCYL        5
#define IDE_SELECT      6
#define IDE_STATUS      7       /* W: Command */
#define IDE_EVEN_DATA   8
#define IDE_ODD_DATA    9

/* Status register values */
#define STATUS_BSY      (1<<7)  /* busy */
#define STATUS_DRDY     (1<<6)  /* device ready */
#define STATUS_DF       (1<<5)  /* device fault */
#define STATUS_DSC      (1<<4)  /* device seek complete */
#define STATUS_DRQ      (1<<3)  /* data request */
#define STATUS_CORR     (1<<2)  /* corrected data (always 0) */
#define STATUS_IDX      (1<<1)  /* index (always 0) */
#define STATUS_ERR      (1<<0)  /* error */

/* IDE commands */
#define CMD_RD_SECTORS  0x20    /* read sector(s) */
#define CMD_IDENTIFY    0xec    /* identify device */

typedef struct IdentifySector {
	u16 config;	// 0
	u16 defcyls;	// 1
	u16 u0;		// 2
	u16 defheads;	// 3
	u16 u1[2];	// 4
	u16 defsectors;	// 6
	
	u32 sectors_per_card;	// 7
	u16 u2;		// 9
	u8 serial[20];	// 10
	u16 u3[2];	// 20
	u16 eccbytes;	// 22
	u8 firmware_rev[8]; // 23
	u8 model[40];	// 27
	
	u16 maxrw;	// 47
	u16 u4;		// 48
	u16 capabilities; // 49
	u16 u5;		// 50
	u16 piomode;	// 51
	u16 dmamode;	// 52
	u16 translation;// 53
	
	u16 cyls;	// 54
	u16 heads;	// 55
	u16 sectors;	// 56
	u16 capacity;	// 57
	u16 u6;
	u16 multimode;
	u32 lbasectors;
	u16 multiwordcap;
	u16 piocap;
	u16 mindmatime;
	u16 recdmatime;
	u16 u7;
	u16 minpiotime;
	u16 u8[14];
	u16 cmdset[6];
	u16 udmacap;
	u16 u9[3];
	u16 apmlvl;
	u16 u10[38];
	u16 features;
	u16 reassigned;
	u16 pwrmode;
	u16 u11[29];
	u16 pwrreq;
	u16 cmdsetext;
	u8 padding[186];
} __attribute__((packed)) IdentifySector;

typedef union
{
        u8 data[512];                   /* Buffer for reading a whole sector at a time  */
        u16 data16[256];                /* 16 bit buffer version.                       */
        u32 data32[128];                /* 32 bit version. Useful for FAT entries       */
} __attribute__((packed)) sector_buffer;

#ifdef FAT_FUNCS
u8 fat_strcmp(u8 * str1, u8 * str2)
{
//      for(;*str1!=0x0 && *str2!=0x00 && *str1==*str2;str1++,str2++);
        while(*str1!=0x0 && *str2!=0x0)
        {
                if(*str1 != *str2) return str1-str2;
                str1++; str2++;
        }
        if(*str1) return -(*str2);
        else return *str1;

}

u8 fat_strlen(u8 * str)
{
        u8 len=0;
        while(str[len] != 0x00) {PRINTF("%c:",str[len]); len++;}
        //while(str[len]!=0x00) { len++; }
        return len;
}

void fat_memcpy(void * dst,void * src,u32 len)
{
        if(len<=0) return;
        u8 * d=(u8 *)dst;
        u8 * s=(u8 *)src;
        while(len--) *(d++) = *(s++);
}
#else
#include "../string.h"
#define fat_strcmp(x,y) (u8)(strcmp( (const char*)(x), (const char*)(y)))
#define fat_strlen(x) (u8)(strlen( (const char*)(x) ))
#define fat_memcpy(x,y,z) memcpy(x,y,z)
#endif

#endif
