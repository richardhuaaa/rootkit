#makefile with support for multiple files for kernel moduels based on http://www.paguilar.org/blog/?p=7

PWD = $(shell pwd)
moduleName = blank
obj-m += $(moduleName).o
blank-objs += main.o moduleHide.o outputDevice.o logInput.o 

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
