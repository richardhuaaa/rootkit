#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/module.h>
#include "communication.h"
#include "common.h"

//TODO: ensure its possible to communicate with the rootkit without being root


// based on http://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module

// Constants
#define PARENT_PROC_ENTRY NULL

static const char *PROC_FILE_NAME = "proc_file_name";

mode_t mode = 0222; // write only

// Function prototyps
static ssize_t receiveWrite(struct file *, const char __user *, size_t, loff_t *);

//Variables
static struct proc_dir_entry *proc_file_entry;

//TODO: test continually using the entry in proc e.g. cat it while uninstalling the rookit

static const struct file_operations proc_file_fops = {
	//TODO: fill these in
	.owner = THIS_MODULE,
	.write = receiveWrite,
	//.open  = open_callback,
	//.read  = read_callback,

};


int communication_init(void) {
	proc_file_entry = proc_create(PROC_FILE_NAME, mode, PARENT_PROC_ENTRY, &proc_file_fops);

	if (proc_file_entry == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void communication_exit(void) {
	remove_proc_entry(PROC_FILE_NAME, PARENT_PROC_ENTRY);
}

static ssize_t receiveWrite(struct file *file, const char *buff, size_t len, loff_t *off) {
	printInfo("<1>Sorry, this operation isn't supported.\n");
	return -EINVAL;
}

