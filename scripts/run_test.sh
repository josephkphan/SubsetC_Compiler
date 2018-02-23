#!/bin/bash

# Created by Joseph Phan
# To use: make sure you have an "examples" directory in the directory with the executable ./scc
cd ./workspace/$1
make
for file in examples/*.c;
do
    filename=${file%.c}
    echo -e "================= Getting Diffs ${filename} ===================="
    # ./scc < ${filename}.c > ${filename}OUT.txt 2>&1
    ./scc < ${filename}.c 2> ${filename}ERROR.txt 
done

echo "================= Getting Diffs ===================="
for file in examples/*.c;
do
    filename=${file%.c}
    echo -e "\n\nRunning Diff Against:" ${filename}    
    echo " --------- DIFF  ${filename} "
	diff ${filename}.err ${filename}ERROR.txt
    echo " --------- END   DIFFS  "
    
done
make clobber
make clean
# rm examples/*.txt