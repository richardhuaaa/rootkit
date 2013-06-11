// based on http://www.faqs.org/docs/kernel/x571.html /  http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html

//TODO: increase buffer size / use tty..

#include <linux/module.h>   // For modules
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <asm/uaccess.h>  //for put_user

#include "messagesToUser.h"

#include "common.h"
#include "communicationOutput.h"
#include "buffer/buffer.h"

#define ERROR_CREATING_OUTPUT_DEVICE_RETURN_VALUE 1 // can not be 0


// Function prototypes

static struct buffer bufferOfDataWaitingToBeSentToUser_;
static Buffer bufferOfDataWaitingToBeSentToUser = &bufferOfDataWaitingToBeSentToUser_;

// Functions
void addCharacterToOutputDevice(char ch) {
	addToBuffer(bufferOfDataWaitingToBeSentToUser, ch);
}

void addStringToOutputDevice(char *str) {
	while (*str != '\0') {
		addCharacterToOutputDevice(*str);
		str++;
	}
}


/* Called when a process, which already opened the dev file, attempts to
	read from it.
*/
ssize_t sendOutputToUser(struct file *filp, char *userBuffer, size_t length, loff_t *offset) {
	int bytes_read = 0; // Number of bytes actually written to the buffer (TODO: rename this)
	
	char ch = getAndRemoveFromBuffer(bufferOfDataWaitingToBeSentToUser);

	/* Actually put the data into the buffer */
	while (length != 0  && ch != '\0')  {
		/* The buffer is in the user data segment, not the kernel segment;
		* assignment won't work.  We have to use put_user which copies data from
		* the kernel data segment to the user data segment. */
		
		put_user(ch, userBuffer);
		
		userBuffer++;
		length--; //TODO: merge with bytes read
		bytes_read++;
		
		ch = getAndRemoveFromBuffer(bufferOfDataWaitingToBeSentToUser);
	}

	/* Most read functions return the number of bytes put into the buffer */
	return bytes_read;
}
