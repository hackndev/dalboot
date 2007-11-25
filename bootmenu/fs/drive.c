#include "../bootmenu.h"

static volatile u8 *io_reg = (u8*) (PALMLD_IDE_PHYS + 0x10);
static IdentifySector cardinfo;

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
                readsw((void*)io_reg + IDE_DATA, buffer, 256);
                swapbytes((u16*)cardinfo.serial, 10);
                swapbytes((u16*)cardinfo.model, 20);
        } else {
                printf("Read error\n");
        }
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
                        readsw((void*)io_reg + IDE_DATA, buf+SECTOR_SIZE*i, 256);
                } else {
                        puts("Read error\n");
                }
        }

        puts("done\n");
}
//void read(u32 start, int count, void *buf);
/*
 * High-level reading of sectors.
 * Reads only the third partition, the one we care about.
 */
void read(u32 start, int count, void * buf)
{
        read_sectors(start+179326,count,buf);
}

void read_sector(u32 start, void * buf)
{
        read_sectors(start, 1, buf);
}

void init_drive()
{
        pxa_gpio_mode(GPIO_NR_PALMLD_IDE_PWEN_MD);
        SET_PALMLD_GPIO(IDE_PWEN, 1);
}

void test_drive()
{
	printf("[IDE] ");
        identify();
        printf("Size: %u MiB CHS: %u/%u/%u \n%.40s\n",
                cardinfo.cyls*cardinfo.heads*cardinfo.sectors/1024/2,
                cardinfo.cyls, cardinfo.heads, cardinfo.sectors,
                                                cardinfo.model);
        u8 mbr[SECTOR_SIZE];
        /* read master boot record */
        read_sectors(0, 1, mbr);
        printf("MBR: ");
        int i;
        for (i=0x1be; i<SECTOR_SIZE; i++)
                printf("%x ", mbr[i]);
        print("\n");
	wait_input();
}

void test_fat()
{
	//test_drive();
	//identify();
	//view_mbr();
	//view_bootsector();
	test_fs_driver();
}
