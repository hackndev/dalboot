#include "fakestdio.h"

extern u8 sector_size_shift;

int getc(FILE * fp)
{
	int c=-1;
	if((fp->offset<<sector_size_shift) + (fp->ptr-fp->base)
				> fp->entry->file_size)
		return -1;
	if(fp->count>0)
	{
		fp->count--;
		c=(int)*(fp->ptr++);
	}else{
		c=fs_fillbuf(fp);
	}

	return c;
}

int feof(FILE * fp)
{
	return ((fp->offset<<sector_size_shift) + (fp->ptr-fp->base)
                                > fp->entry->file_size)?1:0;
}

int fseek(FILE * fp, int offset, int origin)
{
	if(origin == SEEK_END) return -1; //un-implemented
	if(origin == SEEK_SET)
	{
		if((fp->offset<<sector_size_shift) + (fp->ptr-fp->base)==offset) return 0;//seeking to same place
		fp->offset=offset>>sector_size_shift;//offset/512, number of sectors to skip
		fp->ptr=fp->base+(offset&TO_MOD(1<<sector_size_shift)); //offset%sector_size
		fp->count=0;
		return 0;
	}
	if(origin == SEEK_CUR)
	{
		fp->offset+=offset>>sector_size_shift;
		fp->ptr+=offset&TO_MOD(1<<sector_size_shift);
		if(fp->ptr-fp->base>=(1<<sector_size_shift))
		{
			fp->ptr-=1<<sector_size_shift;
			fp->offset++;
		}

		if(offset<fp->count)
			fp->count-=offset;
		else
			fp->count=0;
		return 0;
	}
}

int fgetc(FILE * fp)
{
	return (getc(fp));
}

int fs_fillbuf(FILE * fp)
{
	if(fp->base==NULL)
		fp->base = (char *)malloc(512);
	fp->ptr=fp->base;
	fp->count=fat32_read_file(fp,fp->offset<<sector_size_shift,512,(u8*)fp->base);
	if(fp->count==-1) return -1;
	fp->count--;
	return *fp->ptr++;
}

char * fgets(char * str, int num, FILE * fp)
{
	char * os = str;
	while (--num>0 && (*str=fgetc(fp))!=(char)-1 )
	{
		if(*str=='\n')	break;
		else		str++;
	}
	if(*str==(char)-1) { //EOF
		if(str<=os) return 0; //nothing was left
		else *str='\0'; //no newline at end of file
	}else
		*(str+1)='\0';
	return os;
}
