// TODO: separate part which puts data to this device e.g. add to buffer etc.. which is planned to be here.


// based on http://www.faqs.org/docs/kernel/x571.html /  http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html

/// is a tty device different / ...? e.g. what if the user presses back space / will things be displayed in real time / will it matter...?


#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <asm/uaccess.h>  /* for put_user */
//#include <linux/tty.h>
//#include <linux/tty_driver.h>

#include "constants.h"
#include "outputDevice.h"


// Function prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define CLASS_NAME "outputDeviceClass"
#define DEVICE_NAME_IN_DEV_DIR DEVICE_NAME

static char msg[] = "hello world!\n";
static char *msg_Ptr;

struct class *outputDeviceClass;
struct device *outputDeviceDevice; // TODO: rename these..


/* Global variables are declared as static, so are global within the file. */

static int Major;            /* Major number assigned to our device driver */
static int isDeviceAlreadyOpenSoFurtherOpensShouldFail = 0;

static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
	.open = device_open,
	.release = device_release
};

//static struct tty_struct ttyInfo;

// Functions
/* 
detection: 
The device will be registered e.g. a slot in the file table will be there. The only way around this is to perhaps use a large enough number random number that is hard to find
Though won't work well..
*/
int outputDevice_init(void) {
	//initialize_tty_struct(&ttyInfo, tty_driver *driver, int idx);
	
	//void tty_buffer_init(struct tty_struct *tty);
	
	
	// Register the character device and get the major descriptor number
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk (KERN_INFO "Registering the character device failed with %d\n", Major);
		return Major;
	}
	
	// create the device class / registering in /dev is done for convience. If wanting to hide the rootkit this might not be done.. Though can hide the file there..
	outputDeviceClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(outputDeviceClass)) {
		return PTR_ERR(outputDeviceClass);
	}
	
	outputDeviceDevice = device_create(outputDeviceClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME_IN_DEV_DIR);
	if (IS_ERR(outputDeviceDevice)) {
		printk(KERN_ERR "failed to create device '%s_%s'\n", CLASS_NAME, DEVICE_NAME); // TODO: add print error function etc..
		return PTR_ERR(outputDeviceDevice);
	}

	/*
	printk("<1>I was assigned major number %d.  To talk to\n", Major);
	printk("<1>the driver, create a dev file with\n");
	printk("'mknod /dev/hello c %d 0'.\n", Major);
	*/
	printk(KERN_INFO "created a device: %s\n", DEVICE_NAME_IN_DEV_DIR);
	//printk("<1>Try various minor numbers.  Try to cat and echo to\n"); // minor numbers are only used for 
	//printk("the device file.\n");


	return 0;
}


// the file may remain until released (see device_release) 
void outputDevice_exit(void) {
	device_destroy(outputDeviceClass, MKDEV(Major, 0));
	class_destroy(outputDeviceClass);
	
	unregister_chrdev(Major, DEVICE_NAME);
}  


// Methods
static int device_open(struct inode *inode, struct file *file) {
	if (isDeviceAlreadyOpenSoFurtherOpensShouldFail) {
		return -EBUSY;
	}
	 
	isDeviceAlreadyOpenSoFurtherOpensShouldFail++;
		
	try_module_get(THIS_MODULE);
	
	msg_Ptr = msg;
	
	return SUCCESS;
}


//  Called when a process closes the device file.
static int device_release(struct inode *inode, struct file *file) {
	isDeviceAlreadyOpenSoFurtherOpensShouldFail --;     /* We're now ready for our next caller */
	// see  http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html  "4.1.4. Unregistering A Device"
	module_put(THIS_MODULE); //  Decrement the usage count, or else once you opened the file, you'll never get get rid of the module. 

	return 0;
}


/* Called when a process, which already opened the dev file, attempts to
	read from it.
*/
static ssize_t device_read(struct file *filp,
	char *buffer,    /* The buffer to fill with data */
	size_t length,   /* The length of the buffer     */
	loff_t *offset)  /* Our offset in the file       */
{
	// Number of bytes actually written to the buffer
	int bytes_read = 0;

	/* If we're at the end of the message, return 0 signifying end of file */
	if (*msg_Ptr == 0) return 0;

	/* Actually put the data into the buffer */
	while (length && *msg_Ptr)  {
		/* The buffer is in the user data segment, not the kernel segment;
		* assignment won't work.  We have to use put_user which copies data from
		* the kernel data segment to the user data segment. */
		
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* Most read functions return the number of bytes put into the buffer */
	return bytes_read;
}


/*  Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
	printk ("<1>Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
