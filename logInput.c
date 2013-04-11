//TODO: stop including these everywhere..
#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros


#include "logInput.h"
#include "outputDevice.h"

int __init logInput_init(void) {
	printk(KERN_INFO "in unimplemented %s\n", __FUNCTION__);
	// unimplemented
	addToOutputDevice("Logging input!");
	return 0;
//blah
}

void __exit logInput_exit(void) {
	 // unimplemented
}
