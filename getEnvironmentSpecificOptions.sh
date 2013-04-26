#!/bin/sh

syscallTableNumber=$(sudo grep sys_call_table /boot/System.map-$(uname -r) | cut -f 1 -d " ")
echo "#define SYSCALL_TABLE ((void *) 0x$syscallTableNumber)"
