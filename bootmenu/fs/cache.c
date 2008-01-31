/* Simple buffering code for the FAT32 driver
 *
 */

#include "cache.h"

//local method declarations
u8 cache_have_sector(u32 sector);
void cache_fill_from(u8 which, u8 * buf);
u8 cache_get(u32 sector);

void cache_init()
{
	PRINT("BUFFER INIT\n");
	u8 cur_buf;
	for(cur_buf=0;cur_buf<CACHE_NUM;cur_buf++)
	{
		cache_which_sector[cur_buf]=0;
		cache_num_reads[cur_buf]=0;
	}
}

void cache_read(u32 start, int count, void * buf)
{
	PRINT("BUFFER READ\n");
	//PRINTF("\tstart: %ld, count: %d, buf: %p\n",start,count,buf);
	u8 which;
	int * needed = (int *)malloc(count*sizeof(int));
	for(which=0;which<count;which++)
		needed[which]=(u8)-1;
	//PRINT("\tRESET NEEDED\n");
	// loop through what we need
	u8 cur_buf;
	for(cur_buf=0;cur_buf<count;cur_buf++)
	{
		//PRINTF("\tLooping through needed sectors (%d)\n",cur_buf);
		//if we have the sector, cache_fill_from it
		which=cache_have_sector(start+cur_buf);
		//PRINTF("\tThe buffer this sector is in: %d\n",which);
		if(which!=(u8)-1) //-1 is invalid
			cache_fill_from(which,(buf+(cur_buf<<9)));//cur_buf<<9 = 512*cur_buf
		//if we don't, store it for later since 
		// the next sectors needed might get removed
		else
			needed[cur_buf] = cur_buf;
		//PRINTF("\tDo we need it? %d\n",needed[cur_buf]);
	}

	//Time to get what's left over
	// loop through what we need
	for(cur_buf=0;cur_buf<count;cur_buf++)
	{
		//PRINTF("\tNeeded sector %d\n",needed[cur_buf]);
		//cache_get them
		//cache_fill_from them next
		if(needed[cur_buf]!=(u8)-1)
			cache_fill_from(cache_get(start+cur_buf),(buf+(cur_buf<<9)));
	}
/*
	//Print out the cool stuff:
	for(cur_buf=0;cur_buf<CACHE_NUM;cur_buf++)
        {
		PRINTF("Cache %d, Sector %ld, Reads %d, Data:",cur_buf,
				cache_which_sector[cur_buf],cache_num_reads[cur_buf]);
		int a;
		for(a=0;a<512;a++)
                	PRINTF("%d:%c\n",a,cache_buffers[cur_buf].data[a]);
        }
*/
}

//Returns the location of the buffer with the needed sector
u8 cache_have_sector(u32 sector)
{
	u8 which;
	//loop through cache_which_sectors
	PRINTF("BUFFER HAVE SECTOR: %ld\n",sector);
	for(which=0;which<CACHE_NUM;which++)
	{
		//return the index of the sector that has it
		if(cache_which_sector[which]==sector)
			return which;
	}
	//or return -1
	return (u8)-1;
}

//Fills the buffer with the requested buffer, and 
// increases the num_reads of the sector
void cache_fill_from(u8 which, u8 * buf)
{
	PRINTF("BUFFER FILL FROM cache %d to %p\n",which,buf);
	//just copy over the data
	fat_memcpy(buf,cache_buffers[which].data, 512);
	cache_num_reads[which]++;
}

//Reads the requested sector into the least-used buffer
u8 cache_get(u32 sector)
{
	PRINTF("BUFFER GET SECTOR %ld\n",sector);
	u8 least_read=0;
	//loop through current buffers
	u8 cur_buf;
	//choose the least used
	for(cur_buf=0;cur_buf<CACHE_NUM;cur_buf++)
	{
		if(cache_num_reads[least_read]>cache_num_reads[cur_buf])
			least_read=cur_buf;
	}
	PRINTF("least read: %d\n",least_read);
	//read this sector into buffer
	read(sector,1,&cache_buffers[least_read]);
	cache_which_sector[least_read] = sector;
	cache_num_reads[least_read] = 0;
	return least_read;
}
