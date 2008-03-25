#ifndef _CONFIG_H
#define _CONFIG_H

#include "fs/fakestdio.h"
#include "fs/fs.h"
#include "main.h"

typedef struct option_ll option_ll;

struct option_ll {
	char * title;
	char * kernel;
	option_ll * next;
};

typedef struct {
	int deflt;
	int timeout;
	option_ll * options;
} config_table;


config_table config;

void parse_config(char * filename);
void addTitle(config_table * config, char * title);
void addKernel(config_table * config, char * kernel);
void addPalmOS(config_table * config);


#endif
