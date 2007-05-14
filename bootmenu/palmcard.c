#include "bootmenu.h"

typedef struct PalmCardHeader {
	u32 boot_branch;
	u32 reset_vector;
	u32 magic;		/* 0xfeedbeef */
	u16 header_version;
	u16 flags;
	unsigned char name[32];
	unsigned char manufacturer[32];
	u16 version;
	u32 creation_date;
	u16 ram_blocks;
	u32 block_list_offset;
	u32 rw_os;
	u32 rw_sz;
	u32 ro_os;
	u32 bigrom_os;
	u32 image_size;
	u32 checksum;
	u32 rw_working_os;
	u32 rw_working_sz;
	u32 hal_os;
	unsigned char pad[130];
} __attribute__((packed)) PalmCardHeader;

static PalmCardHeader *card;

void init_palmcard()
{
	card = (PalmCardHeader*)(RAM_BASE + 0x200000);
//	card = (PalmCardHeader*)malloc(sizeof(PalmCardHeader));
	if (card->magic != 0xfeedbeef) {
		printf("Palmcard not detected (magic=%lx)\n", card->magic);
		card = NULL;
		return;
	}
	//printf("Palmcard detected at %p\n", card);
	//printf("rw_os = %lx, ro_os = %lx", card->rw_os, card->ro_os);
	/* 50616d */
}


