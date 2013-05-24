#include <linux/kernel.h>	 // For KERN_INFO
#include <stdarg.h>
#include "messagesToUser.h"



void printInfo(const char *format, ...) {
	va_list argumentPointer;
	va_start(argumentPointer, format);

	va_end(argumentPointer);

 
 
	printk(KERN_INFO, format);
}
