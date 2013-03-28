// TODO: separate part which puts data to this device e.g. add to buffer etc.. which is planned to be here.
//TODO: look at http://www.drdobbs.com/open-source/loadable-modules-the-linux-26-kernel/184406112 - regarding module_inc_use_count etc
// TODO: automatically create the device label etc using the mknod system call after a successful registration and rm during the call to cleanup_module" - using the mknod system call after a successful registration and rm during the call to cleanup_module.




// based on http://www.faqs.org/docs/kernel/x571.html /  http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html
#include <linux/module.h>   // For modules
#include <linux/kernel.h>   // For KERN_INFO
#include <linux/module.h>

/*
#if defined(CONFIG_MODVERSIONS) && ! defined(MODVERSIONS)
	//#include <linux/modversions.h>
	#include <config/modversions.h>

	#define MODVERSIONS
#endif
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>  /* for put_user */


#include "outputDevice.h"

// Function prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev" /// Dev name as it appears in /proc/devices  

static char msg[] = "hello world!";
static char *msg_Ptr;


/* Global variables are declared as static, so are global within the file. */

static int Major;            /* Major number assigned to our device driver */
static int isDeviceAlreadyOpenSoFurtherOpensShouldFail = 0;

static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
	.open = device_open,
	.release = device_release
};

// Functions
void addToOutputDevice(char *str) {
	//TODO: write this!!!!!!!!!!!!!!!!
}


int outputDevice_init(void) {
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk ("Registering the character device failed with %d\n", Major);
		return Major;
	}

	printk("<1>I was assigned major number %d.  To talk to\n", Major);
	printk("<1>the driver, create a dev file with\n");
		printk("'mknod /dev/hello c %d 0'.\n", Major);
	//printk("<1>Try various minor numbers.  Try to cat and echo to\n"); // minor numbers are only used for 
		printk("the device file.\n");

	return 0;
}


// the file may remain until released (see device_release) 
void outputDevice_exit(void) {
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
