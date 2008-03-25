/* Allows for easy adding of options necessary
 *  for loading the linux kernel
 */

/* Will handle most GRUB config options, including:
 *
 * default
 * timeout
 * title
 * kernel // The filename of a zImage
 * PALMOS // Boots PalmOS (if it exists)
 *
 */

#include "config.h"
#include "ctype.h"

extern long simple_strtol(const char *,char **,unsigned int);

void parse_config(char * filename)
{
	memset(&config,0,sizeof(config));
	FILE * fp = fat32_open_file((u8 *)filename);
	printf("%c%c%c\n",fgetc(fp),fgetc(fp),fgetc(fp));
	char buffer[100];
	char * line=buffer;

	while(fgets(buffer,100,fp))
	{
		while(isspace(*line)) line++;
		if(!strncmp(line,"default",7))
			config.deflt=(int)simple_strtol(line+8,0,10);
		else if(!strncmp(line,"timeout",7))
			config.timeout=(int)simple_strtol(line+8,0,10);
		else if(!strncmp(line,"title",5))
			addTitle(&config,line+6);
		else if(!strncmp(line,"kernel",6))
			addKernel(&config,line+7);
		else if(!strncmp(line,"PALMOS",6))
			addPalmOS(&config);

		line=buffer;
	}

	printf("def: %d, timeout: %d\n",config.deflt,config.timeout);
	option_ll * curopt=config.options;
	while(curopt)
	{
		printf("title: %s\n\tkernel: %s\n",curopt->title,curopt->kernel);
		curopt=curopt->next;
	}


	//fat32_close_file(fp);
}

void addTitle(config_table * config, char * title)
{
	option_ll * new_option = (option_ll *)malloc(sizeof(option_ll));
	new_option->next=config->options;
	config->options=new_option;
	config->options->title=(char *)malloc(strlen(title)*sizeof(char));
	strcpy(config->options->title,title);
}
void addKernel(config_table * config, char * kernel)
{
	config->options->kernel=(char *)malloc(strlen(kernel)*sizeof(char));
	strcpy(config->options->kernel,kernel);
}
void addPalmOS(config_table * config)
{
	config->options->kernel=(char *)malloc(7*sizeof(char));
	strcpy(config->options->kernel,"PALMOS");
}

