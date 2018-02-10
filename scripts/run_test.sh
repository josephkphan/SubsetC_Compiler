#!/bin/bash

# Created by Joseph Phan
# To use: make sure you have an "examples" directory in the directory with the executable ./scc
cd ./workspace/$1
make
for file in examples/*.c;
do
    filename=${file%.c}
    echo -e "\n\nTesting Against:" ${filename}    
    # ./scc < ${filename}.c > ${filename}OUT.txt 2>&1
    ./scc < ${filename}.c 2> ${filename}ERROR.txt
    echo " --------- DIFF  ${filename} "
	diff ${filename}.err ${filename}ERROR.txt
    echo " --------- END   DIFFS  "
    
done
make clobber
make clean
# rm examples/*.txt