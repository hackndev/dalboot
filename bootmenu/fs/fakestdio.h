#ifndef _FAKESTDIO_H
#define _FAKESTDIO_H

#include "fs.h"
#include "../main.h"

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define TO_MOD(a) (~((a)-1))


int fgetc(FILE *fp);
int fs_fillbuf(FILE * fp);
char * fgets(char * str, int num, FILE * fp);
int feof(FILE * fp);
int fseek(FILE * fp, int offset, int origin);

#endif
