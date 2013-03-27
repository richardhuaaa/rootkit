#see https://www.kernel.org/doc/Documentation/kbuild/modules.txt for information on some of the rules e.g. -y option
# seperate files - http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/Documentation/kbuild/modules.txt

MAKEDIR = $(shell pwd)
KDIR = /lib/modules/$(shell uname -r)/build
MODULE = main

#sourceFiles =  logInput.c outputDevice.c  #$(wildcard *.c) # note this is not recursive e.g. a/a.c won't be included while a.c in the main directory will be included #todo: remove main.c/ o from here ...

obj-m += main.o
#main-y := $(patsubst %.c,%.o,$(sourceFiles))


all:	$(obj-m)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) module
	make -C $KDIR M=`pwd` main.c
	
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean  #TODO: test this
install: all
	insmod ./$(MODULE).ko
uninstall:
	rmmod $(MODULE)

