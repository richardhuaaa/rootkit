obj-m += helloWorld.o
MAKEDIR = $(shell pwd)
KMAKEDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KMAKEDIR) M=$(MAKEDIR) modules
clean:
	make -C $(KMAKEDIR) M=$(MAKEDIR) clean

