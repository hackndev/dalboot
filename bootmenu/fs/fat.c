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

sector_buffer * sect_buf;
u16 first_data_sector;
u8 cluster_size_shift;
u8 sector_size_shift;


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
	read(sector,1,sect_buf);
	*buffer = (sect_buf->data32[offset>>5]) &0x0FFFFFFF;
}

void uppercase(char * name)
{
	while(*name!=0x00)
	{
		if(*name>'Z') *name -= 32;

		name++;
	}
}

void basename(char * * name, char * * path)
{
	//find last slash and set it to 0x0
	//set name to point after slash
	char * last_slash = *name;
	*path = *name;

	while(**name!=0x0)
	{
		if(**name=='/') last_slash=*name;
	}

	*last_slash='\0';
	*name=last_slash+1;

	return;
}

char * shortname(char * name)
{
	char * shortname = (char *)malloc(11);
	u8 s,j,last_period=0;
	//get first 8
	for(s=0,j=0;s<8,name[j]!=0x00;s++,j++)
	{
	//change invalid to _
		switch(name[j])
		{
		case '.':
			last_period=j;
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
			s--;
	}
		//if more than 8
	if(s==8 && name[j]!= 0x00 && name[j+1]!=0x00)
	{
		//shorten to 6
		//add ~ as 7th
		shortname[6] = '~';
		//put _ after ~, we don't compare this with ~
		shortname[7] = '_';
	}else if(s<7){	//if 7 or less
		//right-pad with spaces
		for(s++;s<8;s++) shortname[s]=' ';
	}
	//get extension
	for(;name[j]!=0x00;j++)
	{
		if(name[j]==".") last_period=j;
	}

	for(s=8,j=last_period;s<11,name[j]!=0x00;s++,j++)
	{
		switch(name[j])
		{
		case '.':
		case ' ':
			s--;
			continue;
			break;
		}
	//uppercase it
		if(name[j]>='a' && name[j]<='z') shortname[s]=name[j]-32;
		else if(name[j]>='A' && name[j]<='Z') shortname[s]=name[j];
		else s--;
	}
	//right-pad with spaces
	if(s<11) for(;s<11;s++) shortname[s]=' ';
	return shortname;
}

void replace_slashes(char * path) //replaces all forward slashes (/) with 0x0
{
	for(;path!=0x0;path++)
	{
		if(*path=='/') *path=0x00;
	}
}

FILE * fat32_open_file(char * name)
{
//	uppercase(name);
	//parse name for directory
	char * path;
	basename(&name,&path); //sets name and path to "path0x0name" where the last / becomes 0x0
	char * shortname = shortname(name);
	//go through FAT32 for the directory
	replace_slashes(path);
	
	//loop through each directory
		//first string starts at 1
		//at the end, set path to (path+strlen(path)+1)
		//test should fail when path==name

	//loop through directory entries
		//until entry > fffffff8

	//loop through sectors of directory for next place in path (path+strlen(path)+1)
		//test against short name



	

	//save the cluster of the directory that the last one was found it
		//and where it was found into the FILE handle

	return 0;
}

void fat32_init()
{
	sect_buf = (sector_buffer *)malloc(sizeof(sector_buffer));
	boot = (bootsector *)malloc(sizeof(bootsector));
	read(0,1,boot);

	first_data_sector = boot->reserved_sectors + boot->num_fat * boot->fat_size;

	cluster_size_shift = 2;
	while(1<<cluster_size_shift != boot->cluster_size && cluster_size_shift<(sizeof(cluster_size_shift)<<3))
		cluster_size_shift++;
	sector_size_shift = 1;
	while(1<<sector_size_shift != boot->sector_size && sector_size_shift<(sizeof(boot->sector_size)<<3))
		sector_size_shift++;

}

void fat32_read_file(FILE * file, u32 start, u32 length, u8 * buffer)
{
//	fat_dir_entry * root_dir;// = (fat_dir_entry *)malloc(sizeof(fat_dir_entry));
//	u32 sector=fat_cluster_to_sector(boot->root_cluster);
//	read(sector,1,sect_buf);
//	root_dir = (fat_dir_entry *)sect_buf->data;
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
	Partition pentry[4] GCC_OPTION(packed);
	u8 magic1; /* 0xaa */
	u8 magic2; /* 0x55 */
} PartitionTable;


union MBRData
{
	PartitionTable table;
	u8 data[512];
};

void view_mbr()
{
	union MBRData *mbr = malloc(sizeof(union MBRData));
	read_sector(0, mbr);

	print("MBR: ");
	int i;
        for (i=0x1be; i<512; i++)
                printf("%x ", mbr->data[i]);
        print("\n");

	print("\nFancy MBR to follow:\n\n");
	
	short beg_cyl = mbr->table.pentry[2].beginchs[2]+(mbr->table.pentry[2].beginchs[1]&0xC0<<8);
	short beg_hds = mbr->table.pentry[2].beginchs[0];
	short beg_sec = mbr->table.pentry[2].beginchs[1]&0x3f;

	short end_cyl = mbr->table.pentry[2].endchs[2]+(mbr->table.pentry[2].endchs[1]&0xC0<<8);
	short end_hds = mbr->table.pentry[2].endchs[0];
	short end_sec = mbr->table.pentry[2].endchs[1]&0x3f;

	printf("Begin C/H/S: %d/%d/%d\n", beg_cyl,
					beg_hds,
					beg_sec);
	printf("End C/H/S: %d/%d/%d\n", end_cyl,
					end_hds,
					end_sec);
	printf("Sector start: %ld\n", mbr->table.pentry[3].absSectStart);
	print("Pentries?: \n");

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
		printf("%x ", mbr->table.pentry[i].bootflag);
		for(j=0;j<3;j++)
			printf("%x ", mbr->table.pentry[i].beginchs[j]);
		printf("%x ", mbr->table.pentry[i].parttype);
		for(j=0;j<3;j++)
			printf("%x ", mbr->table.pentry[i].endchs[j]);
		printf("%lx ", mbr->table.pentry[i].absSectStart);
		printf("%lx ", mbr->table.pentry[i].partSize);
	        print("\n");
	}
        print("\n");
	printf("Magic numbers (should be 0x55 0xaa): %x %x\n", mbr->table.magic1, mbr->table.magic2);
}


typedef union {
	bootsector boot;
	u8 data[512];
} BOOT;

void view_bootsector()
{
	bootsector * boot;
	fat_fs_info * fs_info;

	sect_buf = (sector_buffer *)malloc(sizeof(sector_buffer));
	print("Malloc worked this time...\n");
	u32 pc;
	asm ("mov %0, pc" : : "r" (pc));
	printf("Got pc: %lx\n",pc);

	boot = (bootsector *)malloc(sizeof(bootsector));
	read(0,1,boot);
	
	printf("Jump to boot code: %x %x %x\n",boot->jump[0],
						boot->jump[1],
						boot->jump[2]);
	printf("OEM name and version: %.8s\n",boot->oem_name);
	printf("Sector size in bytes: %d\n",boot->sector_size);
	printf("Cluster size in sectors: %d\n", boot->cluster_size);
	printf("Reserved sectors: %d\n",boot->reserved_sectors);
	printf("Number of FAT tables: %d\n",boot->num_fat);
	printf("Size of these FAT tables: %ld\n",boot->fat_size);
	printf("Sectors per track: %d\n",boot->secs_per_track);
	printf("Heads: %d\n",boot->heads);
	printf("Total number of sectors: %ld\n",boot->total_num_secs);
	printf("Cluster of root directory: %ld should be 2\n",boot->root_cluster);
/*
	print("Reserved: ");
	int i;
	for(i=0;i<12; i++)
		printf("%x ",boot->reserved[i]);
	print("\n");
*/
	printf("Volume label: %.11s\n",boot->volume_lab);
	printf("Filesystem type: %.8s\n",boot->filesystem_type);
	
	first_data_sector = boot->reserved_sectors + boot->num_fat * boot->fat_size;
	printf("First data sector: %d\n",first_data_sector);

	cluster_size_shift = 2;
	while(1<<cluster_size_shift != boot->cluster_size && cluster_size_shift<(sizeof(cluster_size_shift)<<3))
		cluster_size_shift++;
	printf("2^%d = %d or %d\n", cluster_size_shift, 1<<cluster_size_shift, boot->cluster_size);
	if(cluster_size_shift==(sizeof(cluster_size_shift)<<3)) return;

	sector_size_shift = 1;
	while(1<<sector_size_shift != boot->sector_size && sector_size_shift<(sizeof(boot->sector_size)<<3))
		sector_size_shift++;
	printf("2^%d = %d or %d\n", sector_size_shift, 1<<sector_size_shift, boot->sector_size);
	if(sector_size_shift==(sizeof(boot->sector_size)<<3)) return;

	u32 cluster_to_check = 2;
	printf("Sector number of cluster %ld: %ld\n",cluster_to_check,cluster_to_sector(cluster_to_check));
	
	u32 data_sectors = boot->total_num_secs - (boot->reserved_sectors + (boot->num_fat * boot->fat_size));
	u32 count_of_clusters = data_sectors / boot->cluster_size;

	if(count_of_clusters >= 65525)
		print("We have a FAT32 partition\n");
	else
	{
		printf("Umm, not FAT32???\n Clusters = %ld\n",count_of_clusters);
	}
	
	u32 sector,offset;
	cluster_to_entry(cluster_to_check,&sector,&offset);
	printf("Where is cluster %ld in Fat?\nSector: %ld\t\tOffset: %ld\n", cluster_to_check, sector, offset);

	u32 entry;
	read_fat_entry(sector,offset,&entry);
	printf("What does it say?: %lx\n",entry);
	
	printf("Filesystem info table is here: %d\n",boot->fs_info);
	printf("Root directory cluster: %ld\n",boot->root_cluster);
	
//	fs_info = (fat_fs_info *)malloc(sizeof(fat_fs_info));
	read(boot->fs_info,1,sect_buf);
	fs_info=(fat_fs_info *)sect_buf;
	printf("Leading signature: %lx should be %x\n",fs_info->leading_signature,0x41615252);
	printf("Structure signature: %lx should be %x\n",fs_info->struct_signature,0x61417272);
	printf("Free cluster count: %ld\n",fs_info->free_count);
	printf("Next free cluster: %ld\n",fs_info->next_free);
	printf("Trailing signature: %lx should be %x\n",fs_info->trailing_signature,0xAA550000);
	
	fat_dir_entry * root_dir = (fat_dir_entry *)malloc(sizeof(fat_dir_entry));
	sector=cluster_to_sector(boot->root_cluster);
	read(sector,1,sect_buf);
	root_dir = (fat_dir_entry *)sect_buf->data;
	
	print("Root directory:\n");
	printf("\tName: %.11s\n",root_dir->name);
	printf("\tAttributes: %x\n",root_dir->attribs);
	printf("\tFirst cluster: %lu\n", ENTRY_FIRST_CLUSTER(root_dir));
	printf("\tFile size: %lu\n",root_dir->file_size);
	
/*	fat_cluster_to_entry(boot->root_cluster,&sector,&offset);
	fat_read_fat_entry(sector,offset,&entry);
	printf("Root directory entry: %ld\n",entry);
	fat_cluster_to_entry(entry,&sector,&offset);
	fat_read_fat_entry(sector,offset,&entry);
	printf("Second directory entry: %ld\n",entry);
*/
	cluster_to_entry( ENTRY_FIRST_CLUSTER(root_dir) ,&sector,&offset);
	read_fat_entry(sector,offset,&entry);
	printf("Root Directory first cluster entry: %lx\n",entry);
	
	u32 i=0,a=0;
	u8 sect;
	char * long_entry_name=0;
	for(sect=0;sect<32;sect++)
	{
		printf("Reading sector %d:\n",sect);
		read(cluster_to_sector(boot->root_cluster)+sect,1,sect_buf);
		fat_dir_entry * dir = (fat_dir_entry *)sect_buf;
	
		char name[13];
		for(i=0;i<16;i++)
		{
			if(dir[i].name[0]==0x00) goto last_entry;
			if(dir[i].name[0]==0xE5) { printf("Entry %ld is empty.\n",i); continue; }
			if(dir[i].attribs!=0x0f)
			{
				printf("Entry %ld:\n",i+sect*16);
				printf("\tName: %.11s\n",dir[i].name);
				if(long_entry_name)
				{
					printf("\tLong name: %s\n",long_entry_name);
					long_entry_name=0;
				}
				printf("\tAttributes: %x\n",dir[i].attribs);

//				printf("\t\"NT Reserved\" byte: %x",dir[i].reserved_nt);
				print("\t\"NT Reserved\" byte: ");
				printf_ch_bin(dir[i].lower_case); //prints binary!
				print("\n");
				// If (nt&0x08) then extension is lowercase
				// If (nt&0x10) then shortname is lowercase

				printf("\tFirst cluster: %lu\n", ENTRY_FIRST_CLUSTER((&dir[i])));
				printf("\tFile size: %lu\n",dir[i].file_size);
			}else{
				//Long name entry.
				//Unicode characters with palm means they use every other character.
				fat_long_name_entry * long_name = (fat_long_name_entry *)dir+i;
				if(long_name->ordinal&0x40)
				{
					//print("First long entry");
					long_name->ordinal^=0x40;
					long_entry_name = (char *)malloc(13*long_name->ordinal+1);
					long_entry_name[13*long_name->ordinal] = 0x0;
				}
				//printf("Long entry %d:\n",long_name->ordinal);
				//printf("\tName: %.10s%.12s%.4s\n",long_name->name1,long_name->name2,long_name->name3);
				//printf("\tWidechar name: %.5ls%.6ls%.2ls\n",(wchar_t *)long_name->name1,(wchar_t *)long_name->name2,(wchar_t *)long_name->name3);
				//char * name = (char *)malloc(13);
				for(a=0;a<5;a++) name[a]=long_name->name1[a<<1];
				for(a=5;a<11;a++) name[a]=long_name->name2[(a-5)<<1];
				for(a=11;a<13;a++) name[a]=long_name->name3[(a-11)<<1];
	
				/* Set the appropriate part of long_entry_name to name	*/
				for(a=0;a<13;a++) long_entry_name[(13*(long_name->ordinal-1)) + a] = name[a];
				//memcpy(long_entry_name + 13*long_name->ordinal,name,13);
	//			printf("\tFixed name: %.13s\n",name);
	//			printf("\tLong_entry_name so far: %*s\n",13*long_name->ordinal,long_entry_name);
			}
		}
	}
last_entry:
	print("Read last entry\n");
/*
	fat_dir_entry * dir = root_dir;
	read(sector,1,sect_buf);
	dir = (fat_dir_entry *)sect_buf->data;
	print("Second entry in root directory:\n");
        printf("\tName: %s\n",dir->name);
        printf("\tAttributes: %x\n",dir->attribs);
        printf("\tFirst cluster: %lu\n", (((u32)dir->first_clust_hi)<<16) + dir->first_clust_lo);
        printf("\tFile size: %lu\n",dir->file_size);
*/	
/*
	int i;
        for (i=0; i<80; i++)
                printf("%x ", boot->data[i]);
        print("\n");
*/
}
