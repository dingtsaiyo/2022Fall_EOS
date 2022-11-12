#!/bin/sh

# set -x
# set -e

rmmod -f mydev
insmod mydev.ko

./reader.o 192.168.0.200 1234 /dev/mydev &
./writer.o 00Hank
