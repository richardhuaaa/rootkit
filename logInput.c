#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros
#include "logInput.h"

int __init logInput_init(void) {
  return addToOutputDevice("Logging input!");
}

int __exit logInput_exit(void) {
    return 0; // unimplemented
}
