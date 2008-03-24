#ifndef _FAKESTDIO_H
#define _FAKESTDIO_H

#include "fs.h"
#include "../main.h"

int fgetc(FILE *fp);
int fs_fillbuf(FILE * fp);
char * fgets(char * str, int num, FILE * fp);

#endif
