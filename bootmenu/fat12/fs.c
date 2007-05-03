/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "dos.h"
#include "fdos.h"

#if (CONFIG_COMMANDS & CFG_CMD_FDOS)



/*-----------------------------------------------------------------------------
 * fill_fs -- Read info on file system
 *-----------------------------------------------------------------------------
 */
static int fill_fs (BootSector_t *boot, Fs_t *fs)
{

    fs -> fat_start = __le16_to_cpu (boot -> nrsvsect);
    fs -> fat_len = __le16_to_cpu (boot -> fatlen);
    fs -> nb_fat = boot -> nfat;

    fs -> dir_start = fs -> fat_start + fs -> nb_fat * fs -> fat_len;
    fs -> dir_len = __le16_to_cpu (boot -> dirents) * MDIR_SIZE / SZ_STD_SECTOR;
    fs -> cluster_size = boot -> clsiz;
    fs -> num_clus = (fs -> tot_sectors - fs -> dir_start - fs -> dir_len) / fs -> cluster_size;

    return (0);
}

/*-----------------------------------------------------------------------------
 * fs_init --
 *-----------------------------------------------------------------------------
 */
int fs_init (Fs_t *fs)
{
    BootSector_t *boot;

    print("dev_open()\n");
    /* Initialize physical device                                            */
    if (dev_open () < 0) {
	PRINT ("Unable to initialize the fdc\n");
	return (-1);
    }

    print("init_subdir()\n");
    init_subdir ();

    print("malloc() boot sector\n");
    /* Allocate space for read the boot sector                               */
    
    if ((boot = (BootSector_t *)malloc (sizeof (BootSector_t))) == NULL) {
	PRINT ("Unable to allocate space for boot sector\n");
	return (-1);
    }

    print("read boot sector\n");
    /* read boot sector                                                      */
    if (dev_read (boot, 0, 1)){
	PRINT ("Error during boot sector read\n");
	free (boot);
	return (-1);
    }

    //DOS verify does not work on Palm LD since the DOS header of the hard drive is garbage.
    print("DOS verify--bypassed\n");
    /* we verify it'a a DOS diskette                                         
    if (boot -> jump [0] !=  JUMP_0_1 && boot -> jump [0] !=  JUMP_0_2) {
	PRINT ("Not a DOS diskette\n");
	free (boot);
	return (-1);
    }*/

    //media check doesn't work either. Again, the DOS header is garbage.
    print("media check--bypassed\n");
    /*
    if (boot -> descr < MEDIA_STD) {
	 We handle only recent medias (type F8)                            
	PRINT ("unrecognized diskette type\n");
	free (boot);
	return (-1);
    }*/

    //Yep! check_dev sucks too!
    print("check_dev--bypassed\n");
    
    if (check_dev (boot, fs) < 0) {
	PRINT ("Bad diskette\n");
//	free (boot);
//	return (-1);
    }

    print("fill_fs\n");
    if (fill_fs (boot, fs) < 0) {
	PRINT ("fill_fs() failed\n");
	free (boot);

	return (-1);
    }

    print("read_fat\n");
    /* Read FAT                                                              */
    if (read_fat (boot, fs) < 0) {
	PRINT ("read_fat() failed\n");
	free (boot);
	return (-1);
    }

    free (boot);
    return (0);
}

#endif
