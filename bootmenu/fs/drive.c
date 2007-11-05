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

/*
 * Buffering of reads from hard drive
 * Useful for FAT32 driver's consistent reading
 *  from the drive
 */

#define NUM_BUFS 4
u8 buffered_sectors_data[NUM_BUFS][SECTOR_SIZE];
u32 buffered_sectors[NUM_BUFS];
int last_buf_used=-1;

void buffered_read(u32 start, int count, void * buf)
{
	int * to_be_read = (int *)(malloc(count * (sizeof (int))));
	int read_needed=0;
	int cursec;
	for(cursec=0; cursec<count; cursec++)
	{
		int curbuf;
		for(curbuf=0;curbuf<NUM_BUFS; curbuf++)
		{
			if(start + cursec == buffered_sectors[curbuf])
			{
				int i;
				for(i=0;i<SECTOR_SIZE; i++) ((u8 *)buf)[cursec*SECTOR_SIZE+i]=buffered_sectors_data[curbuf][i];

				last_buf_used=curbuf;
				if(last_buf_used==NUM_BUFS-1) last_buf_used=-1;
			}else{
				to_be_read[read_needed++]=cursec;
			}
		}
	}

	//now read the sectors not found in the buffer
	for(cursec=0;cursec<read_needed;cursec++)
	{
		int num_inarow=0;
		while(to_be_read[cursec+1]==to_be_read[cursec]+1) //the next to_be_read is the next sector
		{
			if(num_inarow+cursec+1<NUM_BUFS) //if we have room
				num_inarow++;
			else			//out of room in buffer
				break;
		}
		read(to_be_read[cursec]+start,num_inarow,buffered_sectors_data[cursec]);
		
		buffered_sectors[cursec]=to_be_read[cursec]+start;

		int i;
		for(i=0;i<SECTOR_SIZE; i++) ((u8 *)buf)[to_be_read[cursec]*SECTOR_SIZE+i]=buffered_sectors_data[cursec][i];
	}

	
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
