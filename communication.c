#include <linux/proc_fs.h>
#include "communication.h"

#define PARENT_PROC_ENTRY NULL
static const char *PROC_FILE_NAME = "proc_file_name";

// based on http://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module

struct proc_dir_entry *proc_file_entry;

static const struct file_operations proc_file_fops = {
		//TODO: fill these in
 //.owner = THIS_MODULE,
 //.open  = open_callback,
 //.read  = read_callback,
};


int communication_init(void) {
	proc_file_entry = proc_create(PROC_FILE_NAME, 0, PARENT_PROC_ENTRY, &proc_file_fops);

	if (proc_file_entry == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void communication_exit() {
	remove_proc_entry(PROC_FILE_NAME, PARENT_PROC_ENTRY);
}
