
/* lab6 of LKTAO Task3--print dump_stack
 * To print these infomation, we need to import print_hex_dump in our module
 * Then check kallsyms for the initial address for these buffers
 * 
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");

int init_module()
{
	printk(KERN_ALERT "Dump_stack inserted......\n");
	dump_stack();
	return 0;
}

void cleanup_module()
{
	printk(KERN_INFO "Ending print process.......\n");
}
