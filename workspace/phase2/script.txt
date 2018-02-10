#!/bin/bash

# Created by Joseph Phan
# To use: make sure you have an "examples" directory in the directory with the executable ./scc

for file in examples/*.c;
do
    filename=${file%.c}
    echo "------------------------"
    echo "Testing Against: ${filename}"    
    cp ${filename}.c test.c
    cp ${filename}.out test.out
    ./scc < test.c > test.txt
	diff test.txt test.out
done
rm test.c
rm test.out