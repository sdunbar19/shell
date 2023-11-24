#!/bin/bash

clear
if [ $2 = "clean" ]
then
    make clean
fi
make bin/test_suite_$1
./bin/test_suite_$1