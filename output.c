
/* TODO: remove this file
 * // TODO: rename..#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/tty.h> // TODO: look at /lib/modules/3.2.0-23-generic/build/include/linux/tty.h


/*
TODO: use the tty_buffer to buffer data etc. while dealing with concurrency 

tty_buffer is used by drivers/tty/ehv_bytechan for stdout etc.  using this buffer should result in similar performance 
e.g. same behaviour though operations may be slower
* /
*/

