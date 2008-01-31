#ifndef FAT_CACHE_H
#define FAT_CACHE_H

#include "fat.h"
#include "../bootmenu.h"

#define CACHE_NUM 4
sector_buffer cache_buffers[CACHE_NUM];
u32 cache_which_sector[CACHE_NUM];
u8 cache_num_reads[CACHE_NUM];

void cache_read(u32 start, int count, void * buf);
void cache_init();

#endif
