/* Simple buffering code for the FAT32 driver
 *
 * Based on ARM documentation, a decrementing while loop
 *  is the most efficient in assembly since a subs can be
 *  used instead of a cmp and a sub. Therefore, the searches
 *  in the following code will be from the end to the start.
 */

#include "buffer.h"

//local methods declarations
u8 bfr_have_sector(u32 sector);
void bfr_fill_from(u8 which, u8 * buf);
u8 bfr_get(u32 sector);


void bfr_read(u32 start, int count, void * buf)
{
	u8 which;
	int * needed = (int *)malloc(count*sizeof(int));
	for(which=count;which>0;which--)
		needed[which]=0;
	// loop through what we need
	u8 cur_buf=count;
	do // while(cur_buf-->0)
	{
		//if we have the sector, bfr_fill_from it
		which=bfr_have_sector(start+cur_buf);
		if(which!=(u8)-1) //-1 is invalid
			bfr_fill_from(which,(buf+(cur_buf<<9)));//cur_buf<<9 = 512*cur_buf
		//if we don't, store it for later since 
		// the next sectors needed might get removed
		else
			needed[cur_buf] = cur_buf;
	} while(cur_buf-->0);

	//Time to get what's left over
	// loop through what we need
	cur_buf=count;
	do // while(cur_buf-->0)
	{
		//bfr_get them
		//bfr_fill_from them next
		bfr_fill_from(bfr_get(start+cur_buf),(buf+(cur_buf<<9)));
	} while(cur_buf-->0);


}

u8 bfr_have_sector(u32 sector)
{
	u8 which = BFR_NUM_BFRS;
	//loop through bfr_which_sectors
	do // while(which-->0
	{
		//return the index of the sector that has it
		if(bfr_which_sector[which]==sector)
			return which;
	} while(which-->0);
	//or return -1
	return -1;
}

void bfr_fill_from(u8 which, u8 * buf)
{
	//just copy over the data
	fat_memcpy(buf,bfr_buffers[which].data, 512);
	bfr_num_reads[which]++;
}

u8 bfr_get(u32 sector)
{
	u8 least_read=(u8)-1;
	//loop through current buffers
	u8 cur_buf = BFR_NUM_BFRS;
	while(cur_buf-->0)
	{
		//choose the least used
		if(bfr_num_reads[least_read]>bfr_num_reads[cur_buf])
			least_read=cur_buf;
	}
	//read this sector into buffer
	read(sector,1,&bfr_buffers[least_read]);
	bfr_which_sector[least_read] = sector;
	bfr_num_reads[least_read] = 1;
	return least_read;
}
