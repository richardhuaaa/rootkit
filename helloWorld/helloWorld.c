#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/init.h>     // For init macros

// int init_module(void) {
static int __init helloWorld_init(void) {
  printk(KERN_INFO "Richard was here!\n");
  return 0;
}

// void cleanup_module(void) {
static void __exit helloWorld_exit(void) {
  printk(KERN_INFO "Richard is gone :(\n");
}

module_init(helloWorld_init);
module_exit(helloWorld_exit);

// Get rid of taint message
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Innocent author");
MODULE_DESCRIPTION("This is a perfectly innocent module and has nothing to do with rootkits whatsoever :)");

