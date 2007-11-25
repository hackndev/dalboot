/* Fat.c implements a FAT32 driver.
 * Code derived from dosbox-0.63/src/dos
 *
 */

/* TODO:
 *  init_fat32
 *  open_file
 *  read_file
 *  write_file?
 *  close_file
 *
 */

#include "fat.h"

u16 first_data_sector;
u8 cluster_size_shift;
u8 sector_size_shift;

sector_buffer * sect_buf;
sector_buffer * sect_buf2;
bootsector * boot;


u32 cluster_to_sector(u32 cluster)
{
//	return ((cluster-2) * boot->cluster_size) + first_data_sector;
	return ((cluster-2) << cluster_size_shift) + first_data_sector;
}

void cluster_to_entry(u32 cluster, u32 * sector, u32 * entry_offset)
{
	u32 offset = cluster << 2; // * 4
	*sector = boot->reserved_sectors + (offset>>sector_size_shift);
//	*entry_offset = offset&(0x7F>>(sector_size_shift-1)); //remainder of a bit shift...doesn't work?
	*entry_offset = offset % (1<<sector_size_shift);
}

void read_fat_entry(u32 sector, u32 offset, u32 * buffer)
{
	bfr_read(sector,1,sect_buf);
	*buffer = (sect_buf->data32[offset>>5]) &0x0FFFFFFF;
}

void uppercase(u8 * name)
{
	while(*name!=0x00)
	{
		if(*name>='a' && *name<='z') *name -= 32;

		name++;
	}
}

u8 * basename(u8 * path)
{
	//find last slash and set it to 0x0
	//set name to point after slash
	u8 * last_slash = path;
	u8 * name = path;
	//path = *name;

//	PRINTF("Finding basename of %s\n",*name);
	while(*name!='\0')
	{
//		PRINTF("%x:%c::",**name,**name);
		if(*name=='/') last_slash=name;
		name++;
	}

	*last_slash='\0';
	name=last_slash+1;

	return name;
}


void replace_slashes(u8 * path) //replaces all forward slashes (/) with 0x0
{
	for(;*path!=0x0;path++)
	{
		if(*path=='/') *path=0x00;
	}
}

/* Attempt #2 at opening a file
 * 
 * This one will be much smarter. 
 * Instead of looping through each entry then looking at the long name, this
 * one will loop through the entries, while storing the current long name, 
 * and then when it hits an entry it compares the given name with the short
 * and then long name.
 *
 */
FILE * fat32_open_file(u8 * path)
{
	//Find root directory
	u8 * name;

	FILE * file_handle = (FILE *)malloc(sizeof (FILE));
	file_handle->path=(u8 *)malloc(fat_strlen(path)+1);
	fat_memcpy(file_handle->path,path,fat_strlen(path)+1);
	file_handle->name=basename(file_handle->path);
	PRINTF("file_handle:\n\tpath: %s, name: %s",file_handle->path,file_handle->name);

	name = basename(path);

	if(*path!='\0') //only do this if the path isn't a file in the root
	{
		PRINTF("Replacing slashes in %s\n",path);
		replace_slashes(path); //path now has 0x00 instead of /
	}
	u8 * cur_dir;
	cur_dir=path+1;


	file_handle->entry = (fat_dir_entry *)malloc(sizeof(fat_dir_entry));
	file_handle->entry->first_clust_lo=(boot->root_cluster)&0x0000FFFF;
	file_handle->entry->first_clust_hi=(boot->root_cluster)&0xFFFF0000;
	PRINTF("entry: %ld",ENTRY_FIRST_CLUSTER(file_handle->entry));

	//file_handle->dir_cluster = boot->root_cluster;
	//PRINTF("dir_cluster: %ld should be %ld\n",file_handle->dir_cluster,boot->root_cluster);

	PRINT("Off to the loops we go!\n");
	int cur_sector,cur_entry;
	fat_dir_entry * entry;
	while(cur_dir<=name) //stop after we get past the basename
	{
		PRINTF("Looking for %s\n",cur_dir);
		u8 * long_entry_name=0;
		//loop through the sectors in the cluster
		for(cur_sector=0;cur_sector<boot->cluster_size;cur_sector++)
		{
			//PRINTF("Looking for %s in sector %d (%ld)\n",cur_dir,cur_sector,file_handle->dir_cluster);
			bfr_read(cluster_to_sector(ENTRY_FIRST_CLUSTER(file_handle->entry))+cur_sector,1,sect_buf);
			entry = (fat_dir_entry *)sect_buf;
			//loop through entries in this sector
			//store long entries as they show up
			//when we hit a non-long entry, check the cur_dir versus the short then possible long name
			for(cur_entry=0;cur_entry<boot->sector_size>>5;cur_entry++)
			{
				if(entry[cur_entry].name[0]==0x00) goto not_found;
				if(entry[cur_entry].name[0]==0xE5) continue;
				if(entry[cur_entry].attribs!=0x0f) //normal entry
				{
					//check path against entry[cur_entry].name
					if(fat_strcmp(cur_dir,entry[cur_entry].name)==0) goto found_dir;
					if(long_entry_name)
					{
						//This short name has a long name
						//check path against long_entry_name
						if(fat_strcmp(cur_dir, long_entry_name)==0) goto found_dir;
					}
				}else{ //long name entry, so store it in long_entry_name
					fat_long_name_entry * long_name = (fat_long_name_entry *)entry+cur_entry;
					if(long_name->ordinal&0x40)
					{
						long_name->ordinal^=0x40;
						long_entry_name = (u8 *)malloc(13*long_name->ordinal+1);
						long_entry_name[13*long_name->ordinal] = 0x0;
					}
					short a,add=(13*(long_name->ordinal-1));
					for(a=0;a<5;a++) long_entry_name[add+a]=long_name->name1[a<<1];
					for(a=5;a<11;a++) long_entry_name[add+a]=long_name->name2[(a-5)<<1];
					for(a=11;a<13;a++) long_entry_name[add+a]=long_name->name3[(a-11)<<1];

				}

			}//end for-loop going through the entries in the sector
		}//end for-loop going through the sectors in the cluster
found_dir:
		PRINTF("We founds it!: %s",cur_dir);
		cur_dir=cur_dir+fat_strlen(cur_dir)+1; //points to point in path after this directory
		fat_memcpy(file_handle->entry,entry+cur_entry,sizeof(fat_dir_entry));  //path and name are already set up
	}//end while-loop going through the directories in the path
	
	return file_handle;

not_found:

	return NULL;

}

u8 * shortname(u8 * name)
{

	u8 i,s,j,last_period=0;

	u8 * shortname = (u8 *)malloc(11);

	u8 * ext=0;
	//set ext to last period
	for(i=0;name[i]!=0x00;i++) if(name[i]=='.') ext=&name[i];
	//set last period to null, so name ends with a null before the extension
	if(ext) *ext=0x00;
	//ext now points to extension
	ext++;

	//Set last period to 0x00 so that we only include it once.
	for(s=0; name[s]!=0x00; s++) if(name[s]=='.') last_period=j;
	if(last_period) name[last_period] = 0x00;

	//get first 8
	for(s=0,j=0;s<8 && name[j]!=0x00;s++,j++)
	{
	//change invalid to _
		switch(name[j])
		{
		case '.':
//			last_period=j;
			break;
		case ' ':
			s--;
			continue;
			break;
		}
	//uppercase it
		if(name[j]>='a' && name[j]<='z')
			shortname[s] = name[j]-32;
		else if(name[j]>='A' && name[j] <= 'Z')
			shortname[s] = name[j];
		else
			shortname[s] = '_';
	}
		//if more than 8
	if(s==8 && name[j-1]!= 0x00 && name[j]!=0x00)
	{
		//shorten to 6
		//add ~ as 7th
		shortname[6] = '~';
		//put _ after ~, we don't compare this with ~
		shortname[7] = '_';
	}else if(s<=7){	//if 7 or less
		//right-pad with spaces
		for(;s<8;s++) shortname[s]=' ';
	}
	//get extension
	for(s=8,j=0;s<11 && ext[j]!=0x00;s++,j++)
	{
		switch(name[j])
		{
		case ' ':
			s--;
			continue;
			break;
		}
	//uppercase it
		if(ext[j]>='a' && ext[j]<='z') shortname[s]=ext[j]-32;
		else if(ext[j]>='A' && ext[j]<='Z') shortname[s]=ext[j];
		else s--;
	}
	//right-pad with spaces
	if(s<11) for(;s<11;s++) shortname[s]=' ';
	return shortname;
}

void fat32_init()
{
	boot = (bootsector *)malloc(sizeof(bootsector));
	read(0,1,boot); //the rest gets zeroed with next malloc
	PRINTF("root_cluster: %lu\n",boot->root_cluster);

	sect_buf = (sector_buffer *)malloc(sizeof(sector_buffer));
	sect_buf2 = (sector_buffer *)malloc(sizeof(sector_buffer));

	first_data_sector = boot->reserved_sectors + boot->num_fat * boot->fat_size;
	PRINTF("first data sector: %u\n",first_data_sector);

	cluster_size_shift = 2;
	while(1<<cluster_size_shift != boot->cluster_size && cluster_size_shift<8)
		cluster_size_shift++;
	sector_size_shift = 1;
	while(1<<sector_size_shift != boot->sector_size && sector_size_shift<8)
		sector_size_shift++;

}

u32 fat32_read_file(FILE * file, u32 start, u32 length, u8 * buffer)
{
//	fat_dir_entry * root_dir;// = (fat_dir_entry *)malloc(sizeof(fat_dir_entry));
//	u32 sector=fat_cluster_to_sector(boot->root_cluster);
//	read(sector,1,sect_buf);
//	root_dir = (fat_dir_entry *)sect_buf->data;

	//Get the cluster the file starts at.
	//While start-=cluster_size*sector_size, go to the next cluster
	//	Do this by reading the FAT table for the next one.
	//	If start>0 when FAT entry for cluster is end-of-file, return -1
	//Now that we're in the right starting cluster, while loop through the clusters, and then for through the sectors
	//	In each sector, start-=sector_size, for(sector_size) buffer[curchar*sector_size*cursect]=sect_buf[curchar];
	//	return the number of bytes read
	
	u32 orig_length=length;
	u32 cur_cluster = ENTRY_FIRST_CLUSTER(file->entry);

	//Basically seeking to the right cluster
	u32 entry_sector,entry_offset;
	while(start>(boot->cluster_size<<sector_size_shift)) //means we gotta go to the next cluster
	{
		if(cur_cluster>=0x0ffffff8) //if its the end of the file and we still want to move to the next cluster
			return -1;
		start-=boot->cluster_size<<sector_size_shift;
		cluster_to_entry(cur_cluster,&entry_sector,&entry_offset); //go to next entry
		read_fat_entry(entry_sector,entry_offset,&cur_cluster); // read entry
	}

	//now seek to the right cluster
	u8 cur_sec=0;
	for(;cur_sec<boot->cluster_size && start>(1<<sector_size_shift);cur_sec++)
		start-=1<<sector_size_shift;
	//start reading
	while(cur_cluster<0x0ffffff8 && length>0)
	{
		for(;cur_sec<boot->cluster_size && length>0;cur_sec++)
		{
			read(cluster_to_sector(cur_cluster)+cur_sec,1,sect_buf);
			u16 cur_byte=0;
			while(start-->0)cur_byte++; //seek to the right byte
			for(;cur_byte<boot->sector_size && length>0;cur_byte++)
			{
				buffer[cur_byte+(cur_sec<<sector_size_shift)]=sect_buf->data[cur_byte];
				length--;
			}
		}
		//we went through this cluster and need more
		//set things to the next cluster and continue reading
		cluster_to_entry(cur_cluster,&entry_sector,&entry_offset);
		read_fat_entry(entry_sector,entry_offset,&cur_cluster);
		cur_sec=0;
	}
	return orig_length-length;
}


typedef struct {
	u8 bootflag;
	u8 beginchs[3];
	u8 parttype;
	u8 endchs[3];
	u32 absSectStart GCC_OPTION(packed);
	u32 partSize GCC_OPTION(packed);
} Partition;

typedef struct {
	u8 booter[0x1be];
	Partition pentry[4];
	u8 magic1; /* 0xaa */
	u8 magic2; /* 0x55 */
} GCC_OPTION(packed) PartitionTable;


union MBRData
{
	PartitionTable table;
	u8 data[512];
};

void view_mbr()
{
	union MBRData *mbr = malloc(sizeof(union MBRData));
	read_sector(0, mbr);

	PRINT("MBR: ");
	int i;
        for (i=0x1be; i<512; i++)
                PRINTF("%x ", mbr->data[i]);
        PRINT("\n");

	PRINT("\nFancy MBR to follow:\n\n");
	
	short beg_cyl = mbr->table.pentry[2].beginchs[2]+(mbr->table.pentry[2].beginchs[1]&0xC0<<8);
	short beg_hds = mbr->table.pentry[2].beginchs[0];
	short beg_sec = mbr->table.pentry[2].beginchs[1]&0x3f;

	short end_cyl = mbr->table.pentry[2].endchs[2]+(mbr->table.pentry[2].endchs[1]&0xC0<<8);
	short end_hds = mbr->table.pentry[2].endchs[0];
	short end_sec = mbr->table.pentry[2].endchs[1]&0x3f;

	PRINTF("Begin C/H/S: %d/%d/%d\n", beg_cyl,
					beg_hds,
					beg_sec);
	PRINTF("End C/H/S: %d/%d/%d\n", end_cyl,
					end_hds,
					end_sec);
	PRINTF("Sector start: %ld\n", mbr->table.pentry[3].absSectStart);
	PRINT("Pentries?: \n");

/*
		struct {
			u8 bootflag;
			u8 beginchs[3];
			u8 parttype;
			u8 endchs[3];
			u32 absSectStart;
			u32 partSize;
		} pentry[4];
*/
	int j;
	for (i=0; i<4; i++)
	{
		PRINTF("%x ", mbr->table.pentry[i].bootflag);
		for(j=0;j<3;j++)
			PRINTF("%x ", mbr->table.pentry[i].beginchs[j]);
		PRINTF("%x ", mbr->table.pentry[i].parttype);
		for(j=0;j<3;j++)
			PRINTF("%x ", mbr->table.pentry[i].endchs[j]);
		PRINTF("%lx ", mbr->table.pentry[i].absSectStart);
		PRINTF("%lx ", mbr->table.pentry[i].partSize);
	        PRINT("\n");
	}
        PRINT("\n");
	PRINTF("Magic numbers (should be 0x55 0xaa): %x %x\n", mbr->table.magic1, mbr->table.magic2);
}


typedef union {
	bootsector boot;
	u8 data[512];
} BOOT;

void PRINTF_ch_bin(u8 x)
{
	int n;
	for(n=0; n<8; n++)
	{
		if((x & 0x80) !=0)
		{
			printf("1");
      		}else{
			printf("0");
		}
		if (n==3)
		{
			printf(" "); /* insert a space between nybbles */
		}
		x = x<<1;
	}
}

void test_fs_driver()
{
	PRINT("Initializing fat32 driver...\n");
	fat32_init();
	PRINT("Initialization worked.\n");
	u8 * name = (u8 *)("/dalboot.conf");
	PRINTF("\n--Opening %s:\n",name);
	FILE * file = fat32_open_file(name);
	if(file) {
		PRINT("Whooo! It worked!\n");
		PRINTF("\tDirectory cluster: %ld\n",ENTRY_FIRST_CLUSTER(file->entry));
		PRINTF("\tFile short name: %s\n",file->entry->name);
		PRINTF("\tFile handle path: %s\n",file->path);
		PRINTF("\tFile handle name: %s\n",file->name);
		PRINTF("\tFile entry attributes: %x\n",file->entry->attribs);

		PRINT("\tFile NT reserved: ");
		PRINTF_ch_bin(file->entry->lower_case);
		PRINT("\n");

		PRINTF("\tFile entry file size: %lu\n",file->entry->file_size);
	}else{
		PRINTF("We failed while opening %s...weird\n",name);
		return;
	}
	//file has returned, the file is found
	//now read it...
	u8 * buf = (u8 *)malloc(50);
	u32 length_read;
	length_read = fat32_read_file(file,0,50,buf);
	length_read = fat32_read_file(file,0,50,buf);
	length_read = fat32_read_file(file,0,50,buf);
	PRINTF("Read from the file\n\tHow much: %ld\n\tWhat we read: %s\n",length_read,buf);
}
