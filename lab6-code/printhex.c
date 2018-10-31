
/*
 * lib/hexdump.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

/* lab6 of LKTAO Task1--print cmdline info
 * To print these infomation, we need to import print_hex_dump in our module
 * Then check kallsyms for the initial address for these buffers
 * 
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

//MODULE_LICENSE("GPL");
#define BOOT_CMDLINE_ADDR 0xffffffffab762920
#define SAVED_CMDLINE_ADDR 0xffffffffab8e9008
#define TEST_LEN 10 
int init_module()
{
	const u64 *bootaddr = (void*)BOOT_CMDLINE_ADDR;
//	const u64 *bootaddr = (void*)kallsyms_lookup_name("boot_command_line");
	const u64 *saveaddr = (void*)SAVED_CMDLINE_ADDR;
//	const u64 *saveaddr = (void*)kallsyms_lookup_name("saved_command_line");
	printk(KERN_ALERT "Showing defined cmdline content......\n");
//#define SAVED

#ifdef SAVED
	print_hex_dump(KERN_INFO, "SAVED_CMDLINE_CONTENT:", DUMP_PREFIX_ADDRESS,
			16, 4, (void*)*saveaddr, TEST_LEN, true);
#else
	print_hex_dump(KERN_ALERT, "BOOT_CMDLINE_CONTENT:", DUMP_PREFIX_ADDRESS,
			16, 4, (void*)*bootaddr, TEST_LEN, true);
#endif
	return 0;
}

void cleanup_module()
{
	printk(KERN_INFO "Ending print process.......\n");
}
