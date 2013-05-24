
// see http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#include <linux/kernel.h>	 // For KERN_INFO

// these are included here to ease disabling printing
// macros are used over functions to help work out the actual source of error if printf is called with the wrong arguments in the end

#define printInfo(format, ...) printk(KERN_INFO format, ##__VA_ARGS__)
#define printError(format, ...) printk(KERN_ERR format, ##__VA_ARGS__)
