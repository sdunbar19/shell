#!/bin/bash

if [ $1 = "clean" ]
then
    make clean
fi
clear
make all
./bin/mysh