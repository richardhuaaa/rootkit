MAKEDIR = $(shell pwd)
KMAKEDIR = /lib/modules/$(shell uname -r)/build
MODULE = main logInput

sourceFiles = main.c logInput.c #$(wildcard *.c) # note this is not recursive e.g. a/a.c won't be included while a.c in the main directory will be included

obj-m += $(patsubst %.c,%.o,$(sourceFiles))


all:	$(obj-m)
	make -C $(KMAKEDIR) M=$(MAKEDIR) modules
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean
install: all
	insmod ./$(MODULE).ko
uninstall:
	rmmod $(MODULE)

