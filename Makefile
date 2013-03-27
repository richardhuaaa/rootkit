MAKEDIR = $(shell pwd)
KMAKEDIR = /lib/modules/$(shell uname -r)/build
MODULE = main

obj-m += main.o


all:
	make -C $(KMAKEDIR) M=$(MAKEDIR) modules
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean
install: all
	insmod ./$(MODULE).ko
uninstall:
	rmmod $(MODULE)

