/*
 *  syscall.c
 *
 *  System call "stealing" sample.
 */

/* 
 * Copyright (C) 2001 by Peter Jay Salzman 
 */

/* 
 * The necessary header files 
 */

/*
 * Standard in kernel modules 
 */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module, */
#include <linux/moduleparam.h>	/* which will have params */
#include <linux/unistd.h>	/* The list of system calls */
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/types.h>
/* 
 * For the current (process) structure, we need
 * this to know who the current user is. 
 */
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/cred.h>

#define CR0_WP 0x00010000

/* 
 * The system call table (a table of functions). We
 * just define this as external, and the kernel will
 * fill it up for us when we are insmod'ed
 *  
 * sys_call_table is no longer exported in 2.6.x kernels.
 * If you really want to try this DANGEROUS module you will
 * have to apply the supplied patch against your current kernel
 * and recompile it.
 */
void **sys_call_table;

/* 
 * UID we want to spy on - will be filled from the
 * command line 
 */
static int uid;
module_param(uid, int, 0644);

/* 
 * A pointer to the original system call. The reason
 * we keep this, rather than call the original function
 * (sys_open), is because somebody else might have
 * replaced the system call before us. Note that this
 * is not 100% safe, because if another module
 * replaced sys_open before us, then when we're inserted
 * we'll call the function in that module - and it
 * might be removed before we are.
 *
 * Another reason for this is that we can't get sys_open.
 * It's a static variable, so it is not exported. 
 */
asmlinkage int (*original_call) (int, const char *, int, umode_t);
#define SEARCH_LEN 100

/*
 * To visit and modify sys_call_table, the direct way is to check up
 * /proc/kallsyms and grab the address
 * While this address is dynamic, it is safer to use a function to
 * check the address space, which is not feasible at this time.
 */

#define SYS_CALL_TABLE_ADDR 0xffffffffaae001a0 
#ifndef SYS_CALL_TABLE_ADDR
unsigned long **find_sys_call_table(void)
{
	unsigned long ptr;
	unsigned long *p;
	int i = 0;
	for(ptr = (unsigned long) linux_banner; ; ptr += sizeof(void*)){
		p = (unsigned long *) ptr;
		i += 1;
		if (p[__NR_close] == (unsigned long)sys_close){
			return (unsigned long **)p;
		}
		if (i > SEARCH_LEN) break;
	}
	return NULL;
}
#endif
/* 
 * The function we'll replace sys_open (the function
 * called when you call the open system call) with. To
 * find the exact prototype, with the number and type
 * of arguments, we find the original function first
 * (it's at fs/open.c).
 *
 * In theory, this means that we're tied to the
 * current version of the kernel. In practice, the
 * system calls almost never change (it would wreck havoc
 * and require programs to be recompiled, since the system
 * calls are the interface between the kernel and the
 * processes).
 */
static int cnt = 0;
asmlinkage int our_sys_openat(int pathnm, const char *filename, int flags, int mode)
{
	int i = 0;
	char ch;
	cnt++;
	/* 
	 * Check if this is the user we're spying on 
	 */
	if (uid == current_uid().val) {
		/* 
		 * Report the file, if relevant 
		 */
		printk("Opened file by %d: ", uid);
		do {
			get_user(ch, filename + i);
			i++;
			printk("%c", ch);
		} while (ch != 0);
		printk("\n");
	}

	/* 
	 * Call the original sys_open - otherwise, we lose
	 * the ability to open files 
	 */
	return original_call(pathnm, filename, flags, mode);
}


asmlinkage int openat_time(void)
{
	int res = cnt;
	cnt = 0;
	return res;
}

/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{	
	unsigned long cr0;
      //sys_call_table = (void**) find_sys_call_table();
	sys_call_table = (void**) SYS_CALL_TABLE_ADDR;
	if (!sys_call_table){
		printk(KERN_DEBUG "ERROR: Can not find sys_call_table addr\n");
		return -1;
	}
	printk(KERN_DEBUG "Found sys_call_table at %16lx.\n", (unsigned long)sys_call_table);
	/* 
	 * Warning - too late for it now, but maybe for
	 * next time... 
	 */
	printk(KERN_ALERT "I'm dangerous. I hope you did a ");
	printk(KERN_ALERT "sync before you insmod'ed me.\n");
	printk(KERN_ALERT "My counterpart, cleanup_module(), is even");
	printk(KERN_ALERT "more dangerous. If\n");
	printk(KERN_ALERT "you value your file system, it will ");
	printk(KERN_ALERT "be \"sync; rmmod\" \n");
	printk(KERN_ALERT "when you remove this module.\n");
	cr0 = read_cr0();
	write_cr0(cr0 & ~CR0_WP);
	/* 
	 * Keep a pointer to the original function in
	 * original_call, and then replace the system call
	 * in the system call table with our_sys_open 
	 */
	printk(KERN_ALERT "Get access to sys_call_table.\n");
	original_call = sys_call_table[__NR_openat];
	sys_call_table[__NR_openat] = our_sys_openat;
	/*
	 * No.213 syscall is not implemented, so we use it to return the result
	 */
	sys_call_table[__NR_osf_mvalid] = openat_time;
	printk(KERN_INFO "INSERT COUNTER\n");
	/* 
	 * To get the address of the function for system
	 * call foo, go to sys_call_table[__NR_foo]. 
	 */

	printk(KERN_INFO "Spying on UID:%d\n", uid);
	write_cr0(cr0);
	return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
	unsigned long cr0;
	cr0 = read_cr0();
	write_cr0(cr0 & ~CR0_WP);
	/* 
	 * Return the system call back to normal 
	 */
	if (sys_call_table[__NR_openat] != our_sys_openat) {
		printk(KERN_ALERT "Somebody else also played with the ");
		printk(KERN_ALERT "open system call\n");
		printk(KERN_ALERT "The system may be left in ");
		printk(KERN_ALERT "an unstable state.\n");
	}
	printk(KERN_ALERT "Exiting our module...\n");
	sys_call_table[__NR_openat] = original_call;
	sys_call_table[__NR_osf_mvalid] = NULL;
	write_cr0(cr0);
}
