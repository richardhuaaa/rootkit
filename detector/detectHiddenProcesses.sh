#!/bin/bash

#TODO: use a better number to loop to... e.g. check the maximum number of pids / use the highest pid for some basis

# this may return false postivies if a process just exited..

for pid in {0..10000} ; do
	if [ -e "/proc/$pid" ] ; then
		ls /proc | fgrep "$pid" -w > /dev/null ||  ./displayInfoAboutHiddenPID $pid
	fi
	
	#echo $i
done


