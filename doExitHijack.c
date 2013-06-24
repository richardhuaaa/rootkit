#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include "common.h"
#include "doExitHijack.h"

void (*do_exit_ptr)(long) = (void (*)(long)) DO_EXIT;
char hijackBytes[NUM_HIJACK_BYTES];
char originalBytes[NUM_HIJACK_BYTES];

void replacement_do_exit(long code) {
   printInfo("Entered replacement function\n");
   printInfo("Writing original bytes\n");
   writeHijackBytes(do_exit_ptr, originalBytes, NULL);
   printInfo("Calling original function\n");
   // do_exit_ptr(code);
   do_exit(code);
   printInfo("Writing back hijack function\n");
   writeHijackBytes(do_exit_ptr, hijackBytes, NULL);
   printInfo("Replacement completed successfully\n");
}

void hijack_do_exit() {
   printInfo("Getting hijack bytes\n");
   getHijackBytes(replacement_do_exit, hijackBytes);
   printInfo("Hijacking function\n");
   writeHijackBytes(do_exit_ptr, hijackBytes, originalBytes);
   printInfo("Hijack successful\n");
}

void unhijack_do_exit() {
   printInfo("Unhijacked\n");
   writeHijackBytes(do_exit_ptr, originalBytes, NULL);
}
