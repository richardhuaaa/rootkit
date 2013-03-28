#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros

#include "outputDevice.h"

// #returns ...?
void addToOutputDevice(char *str) {
	printk("%s", str);
}


int __init outputDevice_init(void) {
	printk(KERN_INFO "in unimplemented %s\n", __FUNCTION__);
	
	return 0; // unimplemented
}
int __exit outputDevice_exit(void) {
	return 0; // unimplemented
}


