#based on https://www.kernel.org/doc/Documentation/kbuild/modules.txt

ifneq ($(KERNELRELEASE),)

# kbuild part of makefile
obj-m  := main.o
main-y := logInput.o outputDevice.o moduleHide.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

# Module specific targets
genbin:
	echo "X" > 8123_bin.o_shipped

endif