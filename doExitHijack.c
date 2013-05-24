#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include "common.h"
#include "doExitHijack.h"

void (*do_exit_ptr)(long) = (void (*)(long)) DO_EXIT;
char hijackBytes[NUM_HIJACK_BYTES];
char originalBytes[NUM_HIJACK_BYTES];

void replacement_do_exit(long code) {
   printk("Hijacked\n");
   writeHijackBytes(do_exit_ptr, originalBytes, NULL);
   do_exit_ptr(code);
   writeHijackBytes(do_exit_ptr, hijackBytes, NULL);
}

void hijack_do_exit() {
   getHijackBytes(replacement_do_exit, hijackBytes);
   writeHijackBytes(do_exit_ptr, hijackBytes, originalBytes);
}

void unhijack_do_exit() {
   writeHijackBytes(do_exit_ptr, originalBytes, NULL);
}
