/* tty device
 *  see based on Linux Device Drivers  By Jonathan Corbet, Alessandro Rubini, Greg Kroah-Hartman

*/


/*
 * 
 * Based heavily on :
 * Tiny TTY driver
 *
 * Copyright (C) 2002-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, version 2 of the License.
 *
 * This driver shows how to create a minimal tty driver.  It does not rely on
 * any backing hardware, but creates a timer that emulates data being received
 * from some kind of hardware.
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "outputDevice.h"



#define DRIVER_VERSION "v2.0"
#define DRIVER_AUTHOR "Innocent authors"
#define DRIVER_DESC "Tiny TTY driver"

/* Module information */


#define DELAY_TIME              HZ * 2  /* 2 seconds per character */
#define TINY_DATA_CHARACTER     't'

#define TINY_TTY_MAJOR          240     /* experimental range */
#define TINY_TTY_MINORS         4       /* only have 4 devices */

struct tiny_serial {
        struct tty_struct       *tty;           /* pointer to the tty for this device */
        int                     open_count;     /* number of times this port has been opened */
        struct semaphore        sem;            /* locks this structure */
        struct timer_list       *timer;

        /* for tiocmget and tiocmset functions */
        int                     msr;            /* MSR shadow */
        int                     mcr;            /* MCR shadow */

        /* for ioctl fun */
        struct serial_struct    serial;
        wait_queue_head_t       wait;
        struct async_icount     icount;
};

static struct tiny_serial *tiny_table[TINY_TTY_MINORS]; /* initially all NULL */


static void tiny_timer(unsigned long timer_data)
{
        struct tiny_serial *tiny = (struct tiny_serial *)timer_data;
        struct tty_struct *tty;
        char data[1] = {TINY_DATA_CHARACTER};
        int data_size = 1;

        if (!tiny)
                return;

        tty = tiny->tty;

        /* send the data to the tty layer for users to read.  This doesn't
         * actually push the data through unless tty->low_latency is set */
        tty_buffer_request_room (tty, data_size);
        tty_insert_flip_string(tty, data, data_size);
        tty_flip_buffer_push(tty);

        /* resubmit the timer again */
        tiny->timer->expires = jiffies + DELAY_TIME;
        add_timer(tiny->timer);
}

static int tiny_open(struct tty_struct *tty, struct file *file)
{
        struct tiny_serial *tiny;
        struct timer_list *timer;
        int index;

        /* initialize the pointer in case something fails */
        tty->driver_data = NULL;

        /* get the serial object associated with this tty pointer */
        index = tty->index;
        tiny = tiny_table[index];
        if (tiny == NULL) {
                /* first time accessing this device, let's create it */
                tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
                if (!tiny)
                        return -ENOMEM;

				DEFINE_SEMAPHORE(semaphore);
				tiny->sem = semaphore;
				//__SEMAPHORE_INITIALIZER(tiny->sem, 0);
                //init_MUTEX(&tiny->sem);
                //tiny->open_count = 0;
				
                tiny->timer = NULL;

                tiny_table[index] = tiny;
        }

        down(&tiny->sem);

        /* save our structure within the tty structure */
        tty->driver_data = tiny;
        tiny->tty = tty;

        ++tiny->open_count;
        if (tiny->open_count == 1) {
                /* this is the first time this port is opened */
                /* do any hardware initialization needed here */

                /* create our timer and submit it */
                if (!tiny->timer) {
                        timer = kmalloc(sizeof(*timer), GFP_KERNEL);
                        if (!timer) {
                                up(&tiny->sem);
                                return -ENOMEM;
                        }
                        tiny->timer = timer;
                }
                tiny->timer->data = (unsigned long )tiny;
                tiny->timer->expires = jiffies + DELAY_TIME;
                tiny->timer->function = tiny_timer;
                add_timer(tiny->timer);
        }

        up(&tiny->sem);
        return 0;
}

static void do_close(struct tiny_serial *tiny)
{
        down(&tiny->sem);

        if (!tiny->open_count) {
                /* port was never opened */
                goto exit;
        }

        --tiny->open_count;
        if (tiny->open_count <= 0) {
                /* The port is being closed by the last user. */
                /* Do any hardware specific stuff here */

                /* shut down our timer */
                del_timer(tiny->timer);
        }
exit:
        up(&tiny->sem);
}

static void tiny_close(struct tty_struct *tty, struct file *file)
{
        struct tiny_serial *tiny = tty->driver_data;

        if (tiny)
                do_close(tiny);
}       

static int tiny_write(struct tty_struct *tty, 
                      const unsigned char *buffer, int count)
{
        struct tiny_serial *tiny = tty->driver_data;
        int i;
        int retval = -EINVAL;

        if (!tiny)
                return -ENODEV;

        down(&tiny->sem);

        if (!tiny->open_count)
                /* port was not opened */
                goto exit;

        /* fake sending the data out a hardware port by
         * writing it to the kernel debug log.
         */
        printk(KERN_DEBUG "%s - ", __FUNCTION__);
        for (i = 0; i < count; ++i)
                printk("%02x ", buffer[i]);
        printk("\n");
                
exit:
        up(&tiny->sem);
        return retval;
}

static int tiny_write_room(struct tty_struct *tty) 
{
        struct tiny_serial *tiny = tty->driver_data;
        int room = -EINVAL;

        if (!tiny)
                return -ENODEV;

        down(&tiny->sem);
        
        if (!tiny->open_count) {
                /* port was not opened */
                goto exit;
        }

        /* calculate how much room is left in the device */
        room = 255;

exit:
        up(&tiny->sem);
        return room;
}

#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

static void tiny_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
{
        unsigned int cflag;

        cflag = tty->termios->c_cflag;

        /* check that they really want us to change something */
        if (old_termios) {
                if ((cflag == old_termios->c_cflag) &&
                    (RELEVANT_IFLAG(tty->termios->c_iflag) == 
                     RELEVANT_IFLAG(old_termios->c_iflag))) {
                        printk(KERN_DEBUG " - nothing to change...\n");
                        return;
                }
        }

        /* get the byte size */
        switch (cflag & CSIZE) {
                case CS5:
                        printk(KERN_DEBUG " - data bits = 5\n");
                        break;
                case CS6:
                        printk(KERN_DEBUG " - data bits = 6\n");
                        break;
                case CS7:
                        printk(KERN_DEBUG " - data bits = 7\n");
                        break;
                default:
                case CS8:
                        printk(KERN_DEBUG " - data bits = 8\n");
                        break;
        }
        
        /* determine the parity */
        if (cflag & PARENB)
                if (cflag & PARODD)
                        printk(KERN_DEBUG " - parity = odd\n");
                else
                        printk(KERN_DEBUG " - parity = even\n");
        else
                printk(KERN_DEBUG " - parity = none\n");

        /* figure out the stop bits requested */
        if (cflag & CSTOPB)
                printk(KERN_DEBUG " - stop bits = 2\n");
        else
                printk(KERN_DEBUG " - stop bits = 1\n");

        /* figure out the hardware flow control settings */
        if (cflag & CRTSCTS)
                printk(KERN_DEBUG " - RTS/CTS is enabled\n");
        else
                printk(KERN_DEBUG " - RTS/CTS is disabled\n");
        
        /* determine software flow control */
        /* if we are implementing XON/XOFF, set the start and 
         * stop character in the device */
        if (I_IXOFF(tty) || I_IXON(tty)) {
                unsigned char stop_char  = STOP_CHAR(tty);
                unsigned char start_char = START_CHAR(tty);

                /* if we are implementing INBOUND XON/XOFF */
                if (I_IXOFF(tty))
                        printk(KERN_DEBUG " - INBOUND XON/XOFF is enabled, "
                                "XON = %2x, XOFF = %2x", start_char, stop_char);
                else
                        printk(KERN_DEBUG" - INBOUND XON/XOFF is disabled");

                /* if we are implementing OUTBOUND XON/XOFF */
                if (I_IXON(tty))
                        printk(KERN_DEBUG" - OUTBOUND XON/XOFF is enabled, "
                                "XON = %2x, XOFF = %2x", start_char, stop_char);
                else
                        printk(KERN_DEBUG" - OUTBOUND XON/XOFF is disabled");
        }

        /* get the baud rate wanted */
        printk(KERN_DEBUG " - baud rate = %d", tty_get_baud_rate(tty));
}

/* Our fake UART values */
#define MCR_DTR         0x01
#define MCR_RTS         0x02
#define MCR_LOOP        0x04
#define MSR_CTS         0x08
#define MSR_CD          0x10
#define MSR_RI          0x20
#define MSR_DSR         0x40


static struct tty_operations serial_ops = {
        .open = tiny_open,
        .close = tiny_close,
        .write = tiny_write,
        .write_room = tiny_write_room,
        .set_termios = tiny_set_termios,
};

static struct tty_driver *tiny_tty_driver;

int outputDevice_init(void) {
        int retval;
        int i;

        /* allocate the tty driver */
        tiny_tty_driver = alloc_tty_driver(TINY_TTY_MINORS);
        if (!tiny_tty_driver)
                return -ENOMEM;

        /* initialize the tty driver */
        tiny_tty_driver->owner = THIS_MODULE;
        tiny_tty_driver->driver_name = "tiny_tty";
        tiny_tty_driver->name = "ttty";
        /* no more devfs subsystem */
        tiny_tty_driver->major = TINY_TTY_MAJOR,
        tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
        tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL,
        tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW,
        /* no more devfs subsystem */
        tiny_tty_driver->init_termios = tty_std_termios;
        tiny_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
        tty_set_operations(tiny_tty_driver, &serial_ops);

        /* hack to make the book purty, yet still use these functions in the
         * real driver.  They really should be set up in the serial_ops
         * structure above... */
		/*
        tiny_tty_driver->read_proc = tiny_read_proc;
        tiny_tty_driver->tiocmget = tiny_tiocmget;
        tiny_tty_driver->tiocmset = tiny_tiocmset;
        tiny_tty_driver->ioctl = tiny_ioctl;
		*/

        /* register the tty driver */
        retval = tty_register_driver(tiny_tty_driver);
        if (retval) {
                printk(KERN_ERR "failed to register tiny tty driver");
                put_tty_driver(tiny_tty_driver);
                return retval;
        }

        for (i = 0; i < TINY_TTY_MINORS; ++i)
                tty_register_device(tiny_tty_driver, i, NULL);

        printk(KERN_INFO DRIVER_DESC " " DRIVER_VERSION);
        return retval;
}

void outputDevice_exit(void) {
        struct tiny_serial *tiny;
        int i;

        for (i = 0; i < TINY_TTY_MINORS; i++) {
           tty_unregister_device(tiny_tty_driver, i);
		}
        tty_unregister_driver(tiny_tty_driver);

        /* shut down all of the timers and free the memory */
        for (i = 0; i < TINY_TTY_MINORS; ++i) {
                tiny = tiny_table[i];
                if (tiny) {
                        /* close the port */
                        while (tiny->open_count)
                                do_close(tiny);

                        /* shut down our timer and free the memory */
                        del_timer(tiny->timer);
                        kfree(tiny->timer);
                        kfree(tiny);
                        tiny_table[i] = NULL;
                }
        }
}
