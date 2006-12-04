#include "bootmenu.h"

#define CPU_VENDOR_MASK 0xff000000
#define CPU_MODEL_MASK 0xff000fff

/* CPU Vendors */
#define CPUV_INTEL 0x69000000
#define CPUV_TI 0x54000000
#define CPUV_ARM 0x41000000

/* Intel CPUs */
#define CPU_SA1100 0x69000B11 /* StrongArm */
#define CPU_PXA25X 0x69000010 /* XScale */
#define CPU_PXA27X 0x69000011
#define CPU_PXA210 0x69000012

/* TI CPUs */
#define CPU_915T 0x54000915
#define CPU_925T 0x54000925
#define CPU_926T 0x54000926

/* ARM CPUs */
#define CPU_920T 0x41000920
#define CPU_922T 0x41000922
#define CPU_926E 0x41000926
#define CPU_940T 0x41000940
#define CPU_946E 0x41000946
#define CPU_1020E 0x41000A22

const char *get_cpu_vendor(u32 cpu)
{
	switch (cpu & CPU_VENDOR_MASK) {
	case CPUV_INTEL:
		return "Intel";
	case CPUV_TI:
		return "TI";
	case CPUV_ARM:
		return "Arm";
	}
	return "Unknown";
}

const char *get_cpu_name(u32 cpu)
{
	switch (cpu & CPU_MODEL_MASK) {
	case CPU_920T:
		return "920T";
	case CPU_922T:
		return "922T";
	case CPU_926E:
		return "926E";
	case CPU_940T:
		return "940T";
	case CPU_946E:
		return "946E";
	case CPU_1020E:
		return "1020E";

	case CPU_915T:
		return "915T";
	case CPU_925T:
		return "925T";
	case CPU_926T:
		return "926T";

	case CPU_SA1100:
		return "SA1100";
	case CPU_PXA25X:
		return "PXA25x/26x";
	case CPU_PXA27X:
		return "PXA27x";
	case CPU_PXA210:
		return "PXA210";
	}
	return "???";
}

u32 get_cpu()
{
	u32 cpu = 0;
	u32 id = get_cpu_id();

	if ((id & CPU_VENDOR_MASK) == CPUV_INTEL) {
		cpu |= CPUV_INTEL;

		if (((id >> 4) & 0xfff) == CPU_SA1100) {
			cpu |= CPU_SA1100;
		} else {
			cpu |= (id >> 4) & 31;
		}
	} else {
		cpu |= id & CPU_VENDOR_MASK;
		cpu |= (id >> 4) & 0xFFF;
	}
	return cpu;
}

int cpu_is_pxa()
{
	u32 cpu = get_cpu();
	return cpu == CPU_PXA25X || cpu == CPU_PXA27X || cpu == CPU_PXA210;
}

/*
 * What appears to be a machine code is stored in the IPL in the boot ROM at 
 * address 0x58. This may only be on the newer handhelds - T|T5 and later.
 * 
 * List of codes:
 * 	aAz1 - Tungsten T3 (Arizona)
 * 	ANGS - Tungeten T5 (Angus)
 * 	BRMA - LifeDrive (Brahma)
 *
 */
#define NEW_MACH_OFFSET 0x58
#define OLD_MACH_OFFSET 0x84
u32 get_rom_mach()
{
	u32 code = *(u32*) NEW_MACH_OFFSET;
	/* smallrom in RAM-based devices has the mach code at 0x84, so if we have 
	 * weird characters in our code, use the other offset.
	 */
	while (code) {
		if ((code & 0xff) < ' ' || (code & 0xff) > '~') {
			code = *(u32*) OLD_MACH_OFFSET;
			break;
		}
		code >>= 8;
	}
	return code;
}

void init_mach()
{
	u32 mach = get_rom_mach();

	switch (mach) {
	//case PALMT3: init_palmt3(); break;
	case PALMLD: init_palmld(); break;
	}
}
