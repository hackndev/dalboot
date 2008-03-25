#include "palmld-gpio.h"
#include "main.h"
#include "bootmenu.h"


/**
 * slapin's beautiful malloc implementation.
 */
static void * sbrk = NULL;

void * cheap_malloc(int bytes)
{
//	print ( "cheapo_malloc(): " );
//	printf("sbrk=%lx->",(u32)sbrk);
//	if ( !sbrk ) sbrk = (void *)&_end;
//	printf ("%lx->",(u32)sbrk);
	void * val = sbrk;
	int tmp;
	for(tmp=0;tmp<bytes;tmp++)
	{
//		printf("%lx %x\n",(u32)sbrk,tmp);
//		print(".");
		*((int*)(sbrk+tmp)) = 0;
	}
	sbrk+=bytes;
//	printf("%lx\n",(u32)sbrk);
	return val;
}

extern u32 _start;
extern u32 _end;
extern u32 _length;

int main()
{
//	u32 machcode[2] = {0,0};
	pxa_gpio_mode(GPIO_NR_PALMLD_GREEN_LED_MD);
	SET_PALMLD_GPIO(GREEN_LED, 1);

	init_video();
	fill_screen(RGB16(5,5,5));
	puts("Bootmenu v0.2\nBy Fahrzin Hemmati\n");

	u32 cpu = get_cpu();
	printf("[CPU] %s %s\n", get_cpu_vendor(cpu), get_cpu_name(cpu));
	
	printf("_start: %lx\n_end: %lx\nsbrk: %lx\n",(u32)&_start,(u32)&_end,(u32)sbrk);
	sbrk = (void *)&_end;
	printf("sbrk: %lx\n",(u32)sbrk);

	//machcode[0] = get_rom_mach();
	//printf("[MACH] %s\n", (char*)machcode);

	init_palmld();
	init_palmcard();
	
//	read_a_file("/linux.txt");

	//test_fat();

	parse_config("/boot.cfg");
	display_menu();
	
	
	int k;
	while (1) {
		k = getchar();
		if (k) putchar(k);
		switch (k) {
		case 'h': return 0; /* home: boot palmos */
		}
	}
	return 0;
}



/**
 * Copy bootmenu from wherever it currently is to the
 * _start location defined by the linker.
 *
 * start.S will pass in the current address we're located
 * at and expects the final address of the main function
 * in return.
 */
void *relocate_bootmenu(u32 *src)
{
	u32 * orig_src = src;
	u32 *dest=&_start;
	while (dest < &_end)
		*(dest++) = *(src++);

	//Seems like our code is called when POS goes to sleep
	//Lets set the branch to our code to a 0.
	*orig_src=0;

	return &main;
}

/**
 * Needed by libgcc for div by zero errors etc.
 */
int raise(int sig)
{
	puts("libgcc error. div by zero?\n");
	while(1);
}

/*
	Sets the LED to red while reading from the drive.
*/
void readsw(const void *a, void *b, int c)
{
	SET_PALMLD_GPIO(GREEN_LED, 0);
	SET_PALMLD_GPIO(ORANGE_LED, 1);
	_readsw(a,b,c);
	SET_PALMLD_GPIO(ORANGE_LED, 0);
//	SET_PALMLD_GPIO(GREEN_LED, 1);
}
