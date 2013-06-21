#!/bin/sh

echo "Looking for rootkit in:"
echo "/proc/modules: `grep blank /proc/modules`"

echo "/sys/module: `ls /sys/module | grep blank`"

echo "/proc/kallsyms: `fgrep "[blank]" /proc/kallsyms | head -n1`"
