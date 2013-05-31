#include <linux/fs.h>

int replacement_readdir(struct file *file, void *dirent, filldir_t filldir);
void hijack_readdir(void);
void unhijack_readdir(void);
