#!/bin/bash

#compile

gcc test.c -o test -lhiredis

g++ test.cpp -o test2 -lhiredis

g++ -g -ggdb -fpermissive -Wall -o test3 test3.cpp -L/usr/local/lib -lhiredis

# just to try linker
#g++ -fpermissive -c test3.o test3.cpp 
#ld -o prova test3.o -lhiredis -lc
