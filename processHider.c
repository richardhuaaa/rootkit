#include <linux/pid.h>
#include "processHider.h"


static int hideProcess(int pidNumber);


int processHider_init(void) {
	pid_t pid = 2404; //TODO: change this 
	hideProcess(pid);
	return 0;
}

void processHider_exit(void) {
	//TODO: end the process for real perhaps... (assuming it exists)
}


static int hideProcess(int pidNumber) {
	struct pid *pid = find_get_pid(pidNumber); //todo: check allocation fo this...
	
	printk(KERN_INFO "pid points to %p\n", pid); // TODO: print time etc..
	
	return 0;
}

