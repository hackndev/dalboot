#include "pxa-regs.h"
#include "palmld-gpio.h"
#include "dos.h"
#include "fdos.h"


/* IDE registers */
#define IDE_DATA 	0
#define IDE_ERRPR 	1	/* W: Features */
#define IDE_NSECTOR	2
#define IDE_SECTOR	3
#define IDE_LCYL	4
#define IDE_HCYL	5
#define IDE_SELECT	6
#define IDE_STATUS	7	/* W: Command */
#define IDE_EVEN_DATA	8
#define IDE_ODD_DATA	9

/* Status register values */
#define STATUS_BSY	(1<<7)	/* busy */
#define STATUS_DRDY	(1<<6)	/* device ready */
#define STATUS_DF	(1<<5)	/* device fault */
#define STATUS_DSC	(1<<4)	/* device seek complete */
#define STATUS_DRQ	(1<<3)	/* data request */
#define STATUS_CORR	(1<<2)	/* corrected data (always 0) */
#define STATUS_IDX	(1<<1)	/* index (always 0) */
#define STATUS_ERR	(1<<0)	/* error */

/* IDE commands */
#define CMD_RD_SECTORS	0x20	/* read sector(s) */
#define CMD_IDENTIFY	0xec	/* identify device */

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
static IdentifySector cardinfo;

static volatile u8 *io_reg = (u8*) (PALMLD_IDE_PHYS + 0x10);

static void swapbytes(u16 *buf, int len)
{
	int i;
	for (i=0; i<len; i++)
		buf[i] = ((buf[i] << 8) | (buf[i] >> 8)) & 0xffff;
}

static void identify()
{
	u16 *buffer = (u16*) &cardinfo;
	io_reg[IDE_STATUS] = CMD_IDENTIFY;
	while (io_reg[IDE_STATUS] & STATUS_BSY); //LED is orange
	if (io_reg[IDE_STATUS] & STATUS_DRQ) {
		readsw_led((void*)io_reg + IDE_DATA, buffer, 256);
		swapbytes((u16*)cardinfo.serial, 10);
		swapbytes((u16*)cardinfo.model, 20);
	} else {
		printf("Read error\n");
	}
}

void readsw_led(const void *a, void *b, int c)
{
	SET_PALMLD_GPIO(GREEN_LED, 0);
	SET_PALMLD_GPIO(ORANGE_LED, 1);
	readsw(a,b,c);
	SET_PALMLD_GPIO(GREEN_LED, 1);
	SET_PALMLD_GPIO(ORANGE_LED, 0);
}

static void read_sectors(u32 start, int count, void *buf)
{
	int i;
	printf("reading: %lu %d\n", start, count);
	io_reg[IDE_NSECTOR] = count & 0xff;
	io_reg[IDE_SECTOR] = start & 0xff;
	io_reg[IDE_LCYL] = (start >> 8) & 0xff;
	io_reg[IDE_HCYL] = (start >> 16) & 0xff;
	io_reg[IDE_SELECT] = ((start >> 24) & 0xf) | 0xe0;
	io_reg[IDE_STATUS] = CMD_RD_SECTORS;

	puts("waiting for busy\n");
	while (io_reg[IDE_STATUS] & STATUS_BSY); //LED is orange
	
	puts("reading\n");
	for (i=0; i<count; i++) {
		printf("Count: %d\n", i);
		if (io_reg[IDE_STATUS] & STATUS_DRQ) {
			print ("io_reg[IDE_STATUS] is good\n");
			readsw_led((void*)io_reg + IDE_DATA, buf+512*i, 256);
		} else {
			puts("Read error\n");
		}
	}

	puts("done\n");
}

static int disk_pointer=0;
int fdc_fdos_seek (int where) {
	disk_pointer = where;
	return 1;
}
int fdc_fdos_read (void *buffer, int len) {
	read_sectors(disk_pointer + 179326, len, buffer);
	return 1;
}

void init_ide()
{
	pxa_gpio_mode(GPIO_NR_PALMLD_IDE_PWEN_MD);
	SET_PALMLD_GPIO(IDE_PWEN, 1);
	printf("[IDE] ");
	identify();
	printf("Size: %u MiB CHS: %u/%u/%u \n%.40s\n", 
		cardinfo.cyls*cardinfo.heads*cardinfo.sectors/1024/2, 
		cardinfo.cyls, cardinfo.heads, cardinfo.sectors,
						cardinfo.model);
	u8 mbr[512];
	/* read master boot record */
	read_sectors(0, 1, mbr);
	printf("MBR: ");
	int i;
	for (i=0x1be; i<512; i++)
		printf("%x ", mbr[i]);
	print("\n");
	
}

void read_a_file(char * name)
{
	printf("Starting to open file %s\n",name);

	Fs_t * fs;
	int tmp=fs_init(fs);
	if (tmp) { //Hangs here
		printf("fs_init errored: %u\n",tmp);
		return;
	}
	print("Innited Fs_t fs\n");

	File_t * file;
	file->fs=fs;
	file->name=name;
	
	print("Opening sub directory... ");

	open_subdir(file);
	
	print("Opened sub directory!\n");

	Directory_t * dir;
	Slot_t slot=file->file;	

	print("Opening file...");

	open_file(&slot,dir);
	
	print("File opened!\n");
	print("Reading file...\n");

#define READBYTES	64
	char buffer[READBYTES];
//	skip first byte because it gives us errors and doesn't get shown anyway
	read_file(fs,&slot,buffer,1,READBYTES);
	printf("First %d chars of %s is :%s:", READBYTES, name, buffer);
}
