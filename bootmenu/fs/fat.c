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
	read(sector,1,sect_buf);
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

u8 fat_strcmp(u8 * str1, u8 * str2)
{
//	for(;*str1!=0x0 && *str2!=0x00 && *str1==*str2;str1++,str2++);
	while(*str1!=0x0 && *str2!=0x0)
	{
		if(*str1 != *str2) return str1-str2;
		str1++; str2++;
	}
	if(*str1) return -(*str2);
	else return *str1;
		
}

u8 fat_strlen(u8 * str)
{
	u8 len=0;
	while(str[len] != 0x00) {PRINTF("%c:",str[len]); len++;}
	//while(str[len]!=0x00) { len++; }
	return len;
}

void fat_memcpy(void * dst,void * src,u32 len)
{
	if(len<=0) return;
/*
	register char * d;
	register char * s;

	d=(char *)dst;
	s=(char *)src;

	while(len>=4)
	{
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		len-=4;
	}
*/
	u8 * d=(u8 *)dst;
	u8 * s=(u8 *)src;
	while(len--) *(d++) = *(s++);
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
			read(cluster_to_sector(ENTRY_FIRST_CLUSTER(file_handle->entry))+cur_sector,1,sect_buf);
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
/*
	u8 i,j=0;
	u8 * ext=0;
	//set ext to last period
	for(i=0;name[i]!=0x00;i++) if(name[i]=='.') ext=&name[i];
	//set last period to null, so name ends with a null before the extension
	*ext=0x00;
	//ext now points to extension
	ext++;

	u8 * shortname = (u8 *)malloc(11);

	//copy anything over until the first space
	for (i=0;(name[i]!=' ') && (i<8);i++) shortname[i]=name[i];

	//copy over extension
	if (ext && ext[0]!=' ')
	{
		while(ext && j++<3) shortname[8+j]=ext[j];
	}
	
	//I really don't think this works.....
	//Maybe if I add the ext thing to the algorithm below?
	return shortname;
}
*/

/*Will read the dir_entry.
check against the shortname, using the lower_case entry
	return if same
then read the long entries
	if offset==0, read sector-1 into sect_buf2
		using new sect_buf since we need to let the calling function use sect_buf with a clear conscience
	if ordinal==0 or E5, then there is no long_entry. return -1
	keep reading the entry before until we get a ordinal mask of 0x40
	malloc enough space for ordinal&~0x40*13
	read the entries again! but this time, put the odd characters into the malloced space
now compare the new long_entry with the given name
*/
u8 compare_dir_entry_with_name(fat_dir_entry * dir, u8 sector, u8 offset, u8 * name)
{
	u16 i=0;
	if(dir[offset].lower_case&0x10)//name is lower case
	{
		PRINT("name is lower case\n");
		for(i=0;i<8;i++)
			if(dir[offset].name[i]>='a' && dir[offset].name[i]<='z')
				dir[offset].name[i]-=32;
	}
	if(dir[offset].lower_case&0x08)//ext is lower case
	{
		PRINT("ext is lower case\n");
		for(i=8;i<11;i++)
			if(dir[offset].name[i]>='a' && dir[offset].name[i]<='z')
				dir[offset].name[i]-=32;
	}
	
	if(fat_strcmp(name,dir[offset].name)==0) return true;
	
	u8 * long_name;
	fat_long_name_entry * lname;
	//memcpy(sect_buf,sect_buf2,boot->sector_size);
	if(offset>0) for(i=0; i<sizeof(sect_buf); i++) sect_buf2[i]=sect_buf[i];
//	PRINTF("sect_buf is %s\n",sect_buf->data);
//	PRINTF("sect_buf2 is %s\n",sect_buf2->data);

	do
	{ //while(lname->ordinal!=E5 or &0x40
		offset--;
		PRINTF("Offset: %d\n",offset);
		if(offset==((u8)-1))
		{
			offset+=boot->sector_size>>5;// /32 = >> 5
			sector--;
			PRINTF("Offset: %d, Sector: %d\n",offset,sector);
			read(sector,1,sect_buf2);
			lname=(fat_long_name_entry *)(sect_buf2+(offset<<5)); //(offset=15) * 32 = 480
		}else{
			lname=(fat_long_name_entry *)(sect_buf2+(offset<<5)); //offset * 32
		}
		PRINTF("Long entry %d:\n",lname->ordinal);
		PRINTF("\tName: %.10s%.12s%.4s\n",lname->name1,lname->name2,lname->name3);
	} while(lname->attribs==0x0f && lname->ordinal!=0xE5 && (lname->ordinal&0x40)==0);
	//fail if E5 or not a long_entry, or stop and malloc if &0x40
	if(lname->ordinal==0xE5 || (lname->attribs&0x0f)!=0x0f) return false;
	else long_name=(u8 *)malloc(13*(lname->ordinal&(~0x40)));
	PRINTF("Last long entry: %d\n",lname->ordinal&(~0x40));

	
	while(lname->attribs==0x0f)
	{
		//set the right characters in long_name to those of this long_entry
		while(lname->attribs==0x0f)
		{
			//weird math to get the right character into the right place and 
			for(i=0;i<5;i++) long_name[i+13*(lname->ordinal-1)] = lname->name1[i<<1];
			for(i=5;i<11;i++) long_name[i+13*(lname->ordinal-1)] = lname->name2[(i-5)<<1];
			for(i=11;i<12;i++) long_name[i+13*(lname->ordinal-1)] = lname->name3[(i-11)<<1];
			offset++;
			if(offset>(boot->cluster_size>>5))
			{
				offset=0;
				sector++;

				read(sector,1,sect_buf2);
	                        lname=(fat_long_name_entry *)sect_buf2;
	                }else{
	                        lname=(fat_long_name_entry *)(sect_buf2+offset);
			}
		}
	}
	//long_name should now be the file's long name, with space padding. Only test the length of our string
	PRINTF("long_name: %s\nFilename: %s\n",long_name,name);
	return fat_strcmp(long_name,name);
}



FILE * fat32_open_file_old(u8 * name)
{
	FILE * file_handle;

//	uppercase(name);
	//parse name for directory
	u8 * path;
//	u8 boot_cluster_size = boot->cluster_size; //Apparently this gets set to 0 at some point...

	PRINTF("Finding basename of %s\n",name);
	//basename(&name,&path); //sets name and path to "path0x0name" where the last / becomes 0x0
	PRINTF("The path is %s and the name is %s\n",path,name);
//	char * short_name = shortname(name);
	//go through FAT32 for the directory
	PRINT("Replacing slashes in path\n");
	replace_slashes(path);
	PRINT("Path without slashes: ");
	u8 * tpath;
	for(tpath=path;tpath<name;tpath++)
	{
		PRINTF("%c",(*tpath!=0x0)?*tpath:':');
	}
	
	//first name starts at 1
	path++;
	PRINTF("Path without slashes and skipping the first byte: %s\n",path);

	u8 entry_offset=0,cur_sec=0;
	u32 cluster=2;
	//loop through each directory
	while(path<=name)
	{
		//test should fail when path>name

		//loop through directory linked-list
		u32 sector,offset,entry=0;
		cluster=boot->root_cluster;

		while(entry<0x0ffffff8)		//until entry >= 0ffffff8
		{
			PRINTF("Finding entry of cluster %ld:\n",cluster);
			cluster_to_entry(cluster, &sector, &offset);
			PRINTF("\tSector: %ld\n",sector);
			PRINTF("\tOffset: %ld\n",offset);

			read_fat_entry(sector, offset, &entry);
			PRINTF("\tEntry: %lx\n",entry);

			//loop through sectors of directory for next place in path (path+strlen(path)+1)

			for(cur_sec=0; cur_sec < boot->cluster_size; cur_sec++)
			{
				PRINTF("Current sector: %d, cluster size: %d\n",cur_sec, boot->cluster_size);

				//test against long name
				read(cluster_to_sector(cluster)+cur_sec,1,sect_buf);
				fat_dir_entry * dir_entry = (fat_dir_entry *)sect_buf;
				//loop through directory entries
				for(entry_offset=0;entry_offset<16;entry_offset++)
				{
					if(dir_entry[entry_offset].attribs == 0x0f) continue;
					else if(dir_entry[entry_offset].name[0] == 0xE5) continue;
					else if(dir_entry[entry_offset].name[0] == 0x00)
					{
						PRINT("Failed!!!\n");
						goto failed;
					}

					PRINTF("Read directory entry %d:\n",entry_offset+(cur_sec<<4));
					PRINTF("\tName: %.11s\n",dir_entry[entry_offset].name);
					PRINTF("\tAttribs: %x\n",dir_entry[entry_offset].attribs);
					PRINTF("\tFirst cluster: %ld\n",ENTRY_FIRST_CLUSTER((&dir_entry[entry_offset])));

					//Short name is unnecessary. Compare the long names.
					//u8 * short_dir_name = shortname(path);
					//PRINTF("Short name of %s is %s\n",path,short_dir_name);
					u8 comparison = compare_dir_entry_with_name(
								dir_entry,
									cluster_to_sector(cluster)+cur_sec,
										entry_offset,path);
					PRINTF("Comparison returned: %d\n", comparison);
					if(comparison==true)
					//if(fat_strcmp(dir_entry[entry_offset].name,short_dir_name)==0)
					{
						if(path==name)
						{
							PRINT("Found name\n");
							goto found_name;
						}
						else
						{
							PRINT("Found path\n");
							goto found_path;
						}
					}
//					if(strcmp(dir_entry[entry_offset].name,short_name)==0) goto found_file;
				}
			}

found_path:
			
			//read next cluster
			if(entry<0x0ffffff8)
			{
				cluster=entry;
				cluster_to_entry(entry, &sector, &offset);
				read_fat_entry(sector, offset, &entry);	
			}
			PRINTF("entry is %lx\n",entry);
		} //while(entry<0x0ffffff8);

		//at the end, set path to (path+strlen(path)+1)
		path += fat_strlen(path) + 1;

	}
	
	//We failed looking for the file...
failed:
	return 0;

found_name:

	file_handle = (FILE *)malloc(sizeof(FILE));
	//save the cluster of the directory that the last one was found it
	//file_handle->dir_cluster = cluster;
		//and where it was found into the FILE handle
	//file_handle->sector_offset = cur_sec;
	//file_handle->entry_offset = entry_offset;

	return file_handle;
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

	u32 entry_sector,entry_offset;
	while(start>(boot->cluster_size<<sector_size_shift)) //means we gotta go to the next cluster
	{
		if(cur_cluster>=0x0ffffff8) //if its the end of the file and we still want to move to the next cluster
			return -1;
		start-=boot->cluster_size<<sector_size_shift;
		cluster_to_entry(cur_cluster,&entry_sector,&entry_offset); //go to next entry
		read_fat_entry(entry_sector,entry_offset,&cur_cluster); // read entry
	}
	while(cur_cluster<0x0ffffff8 && length>0)
	{
		u8 cur_sec=0;
		for(;cur_sec<boot->cluster_size && length>0;cur_sec++)
		{
			read(cluster_to_sector(cur_cluster)+cur_sec,1,sect_buf);
			u16 cur_byte=0;
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

void view_bootsector()
{
	fat_fs_info * fs_info;

	sect_buf = (sector_buffer *)malloc(sizeof(sector_buffer));
	PRINT("Malloc worked this time...\n");
	u32 pc;
	asm ("mov %0, pc" : : "r" (pc));
	PRINTF("Got pc: %lx\n",pc);

	boot = (bootsector *)malloc(sizeof(bootsector));
	read(0,1,boot);
	
	PRINTF("Jump to boot code: %x %x %x\n",boot->jump[0],
						boot->jump[1],
						boot->jump[2]);
	PRINTF("OEM name and version: %.8s\n",boot->oem_name);
	PRINTF("Sector size in bytes: %d\n",boot->sector_size);
	PRINTF("Cluster size in sectors: %d\n", boot->cluster_size);
	PRINTF("Reserved sectors: %d\n",boot->reserved_sectors);
	PRINTF("Number of FAT tables: %d\n",boot->num_fat);
	PRINTF("Size of these FAT tables: %ld\n",boot->fat_size);
	PRINTF("Sectors per track: %d\n",boot->secs_per_track);
	PRINTF("Heads: %d\n",boot->heads);
	PRINTF("Total number of sectors: %ld\n",boot->total_num_secs);
	PRINTF("Cluster of root directory: %ld should be 2\n",boot->root_cluster);
/*
	PRINT("Reserved: ");
	int i;
	for(i=0;i<12; i++)
		PRINTF("%x ",boot->reserved[i]);
	PRINT("\n");
*/
	PRINTF("Volume label: %.11s\n",boot->volume_lab);
	PRINTF("Filesystem type: %.8s\n",boot->filesystem_type);
	
	first_data_sector = boot->reserved_sectors + boot->num_fat * boot->fat_size;
	PRINTF("First data sector: %d\n",first_data_sector);

	cluster_size_shift = 2;
	while(1<<cluster_size_shift != boot->cluster_size && cluster_size_shift<(sizeof(cluster_size_shift)<<3))
		cluster_size_shift++;
	PRINTF("2^%d = %d or %d\n", cluster_size_shift, 1<<cluster_size_shift, boot->cluster_size);
	if(cluster_size_shift==(sizeof(cluster_size_shift)<<3)) return;

	sector_size_shift = 1;
	while(1<<sector_size_shift != boot->sector_size && sector_size_shift<(sizeof(boot->sector_size)<<3))
		sector_size_shift++;
	PRINTF("2^%d = %d or %d\n", sector_size_shift, 1<<sector_size_shift, boot->sector_size);
	if(sector_size_shift==(sizeof(boot->sector_size)<<3)) return;

	u32 cluster_to_check = 2;
	PRINTF("Sector number of cluster %ld: %ld\n",cluster_to_check,cluster_to_sector(cluster_to_check));
	
	u32 data_sectors = boot->total_num_secs - (boot->reserved_sectors + (boot->num_fat * boot->fat_size));
	u32 count_of_clusters = data_sectors / boot->cluster_size;

	if(count_of_clusters >= 65525)
		PRINT("We have a FAT32 partition\n");
	else
	{
		PRINTF("Umm, not FAT32???\n Clusters = %ld\n",count_of_clusters);
	}
	
	u32 sector,offset;
	cluster_to_entry(cluster_to_check,&sector,&offset);
	PRINTF("Where is cluster %ld in Fat?\nSector: %ld\t\tOffset: %ld\n", cluster_to_check, sector, offset);

	u32 entry;
	read_fat_entry(sector,offset,&entry);
	PRINTF("What does it say?: %lx\n",entry);
	
	PRINTF("Filesystem info table is here: %d\n",boot->fs_info);
	PRINTF("Root directory cluster: %ld\n",boot->root_cluster);
	
//	fs_info = (fat_fs_info *)malloc(sizeof(fat_fs_info));
	read(boot->fs_info,1,sect_buf);
	fs_info=(fat_fs_info *)sect_buf;
	PRINTF("Leading signature: %lx should be %x\n",fs_info->leading_signature,0x41615252);
	PRINTF("Structure signature: %lx should be %x\n",fs_info->struct_signature,0x61417272);
	PRINTF("Free cluster count: %ld\n",fs_info->free_count);
	PRINTF("Next free cluster: %ld\n",fs_info->next_free);
	PRINTF("Trailing signature: %lx should be %x\n",fs_info->trailing_signature,0xAA550000);
	
	fat_dir_entry * root_dir = (fat_dir_entry *)malloc(sizeof(fat_dir_entry));
	sector=cluster_to_sector(boot->root_cluster);
	read(sector,1,sect_buf);
	root_dir = (fat_dir_entry *)sect_buf->data;
	
	PRINT("Root directory:\n");
	PRINTF("\tName: %.11s\n",root_dir->name);
	PRINTF("\tAttributes: %x\n",root_dir->attribs);
	PRINTF("\tFirst cluster: %lu\n", ENTRY_FIRST_CLUSTER(root_dir));
	PRINTF("\tFile size: %lu\n",root_dir->file_size);
	
/*	fat_cluster_to_entry(boot->root_cluster,&sector,&offset);
	fat_read_fat_entry(sector,offset,&entry);
	PRINTF("Root directory entry: %ld\n",entry);
	fat_cluster_to_entry(entry,&sector,&offset);
	fat_read_fat_entry(sector,offset,&entry);
	PRINTF("Second directory entry: %ld\n",entry);
*/
	cluster_to_entry( ENTRY_FIRST_CLUSTER(root_dir) ,&sector,&offset);
	read_fat_entry(sector,offset,&entry);
	PRINTF("Root Directory first cluster entry: %lx\n",entry);
	
	u32 i=0,a=0;
	u8 sect;
	u8 * long_entry_name=0;
	for(sect=0;sect<32;sect++)
	{
		PRINTF("Reading sector %d:\n",sect);
		read(cluster_to_sector(boot->root_cluster)+sect,1,sect_buf);
		fat_dir_entry * dir = (fat_dir_entry *)sect_buf;
	
		u8 name[13];
		for(i=0;i<16;i++)
		{
			if(dir[i].name[0]==0x00) goto last_entry;
			if(dir[i].name[0]==0xE5) { PRINTF("Entry %ld is empty.\n",i); continue; }
			if(dir[i].attribs!=0x0f)
			{
				PRINTF("Entry %ld:\n",i+sect*16);
				PRINTF("\tName: %.11s\n",dir[i].name);
				if(long_entry_name)
				{
					PRINTF("\tLong name: %s\n",long_entry_name);
					long_entry_name=0;
				}
				PRINTF("\tAttributes: %x\n",dir[i].attribs);

//				PRINTF("\t\"NT Reserved\" byte: %x",dir[i].reserved_nt);
				PRINT("\t\"NT Reserved\" byte: ");
				PRINTF_ch_bin(dir[i].lower_case); //PRINTs binary!
				PRINT("\n");
				// If (nt&0x08) then extension is lowercase
				// If (nt&0x10) then shortname is lowercase

				PRINTF("\tFirst cluster: %lu\n", ENTRY_FIRST_CLUSTER((&dir[i])));
				PRINTF("\tFile size: %lu\n",dir[i].file_size);
			}else{
				//Long name entry.
				//Unicode characters with palm means they use every other character.
				fat_long_name_entry * long_name = (fat_long_name_entry *)dir+i;
				if(long_name->ordinal&0x40)
				{
					//PRINT("First long entry");
					long_name->ordinal^=0x40;
					long_entry_name = (u8 *)malloc(13*long_name->ordinal+1);
					long_entry_name[13*long_name->ordinal] = 0x0;
				}
				//PRINTF("Long entry %d:\n",long_name->ordinal);
				//PRINTF("\tName: %.10s%.12s%.4s\n",long_name->name1,long_name->name2,long_name->name3);
				//PRINTF("\tWidechar name: %.5ls%.6ls%.2ls\n",(wchar_t *)long_name->name1,(wchar_t *)long_name->name2,(wchar_t *)long_name->name3);
				//char * name = (char *)malloc(13);
				for(a=0;a<5;a++) name[a]=long_name->name1[a<<1];
				for(a=5;a<11;a++) name[a]=long_name->name2[(a-5)<<1];
				for(a=11;a<13;a++) name[a]=long_name->name3[(a-11)<<1];
	
				/* Set the appropriate part of long_entry_name to name	*/
				for(a=0;a<13;a++) long_entry_name[(13*(long_name->ordinal-1)) + a] = name[a];
				//memcpy(long_entry_name + 13*long_name->ordinal,name,13);
	//			PRINTF("\tFixed name: %.13s\n",name);
	//			PRINTF("\tLong_entry_name so far: %*s\n",13*long_name->ordinal,long_entry_name);
			}
		}
	}
last_entry:
	PRINT("Read last entry\n");
/*
	fat_dir_entry * dir = root_dir;
	read(sector,1,sect_buf);
	dir = (fat_dir_entry *)sect_buf->data;
	PRINT("Second entry in root directory:\n");
        PRINTF("\tName: %s\n",dir->name);
        PRINTF("\tAttributes: %x\n",dir->attribs);
        PRINTF("\tFirst cluster: %lu\n", (((u32)dir->first_clust_hi)<<16) + dir->first_clust_lo);
        PRINTF("\tFile size: %lu\n",dir->file_size);
*/	
/*
	int i;
        for (i=0; i<80; i++)
                PRINTF("%x ", boot->data[i]);
        PRINT("\n");
*/
}


void test_fs_driver()
{
	PRINT("Initializing fat32 driver...\n");
	fat32_init();
	PRINT("Initialization worked.\n");
	u8 * name = (u8 *)("/cocoboot.conf");
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
	u8 * buf = (u8 *)malloc(10);
	u32 length_read = fat32_read_file(file,0,30,buf);
	PRINTF("Read from the file\n\tHow much: %ld\n\tWhat we read: %s\n",length_read,buf);
}
