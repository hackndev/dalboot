#include "../bootmenu.h"

#define BFR_NUM_BFRS 4
sector_buffer bfr_buffers[BFR_NUM_BFRS];
u32 bfr_which_sector[BFR_NUM_BFRS];
u8 bfr_num_reads[BFR_NUM_BFRS];

void bfr_read(u32 start, int count, void * buf);
