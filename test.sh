#!/bin/bash-e
echo "Compiling"
gcc vm.c -o vm
echo "Running vm"
./vm -n 128 -p lru >out.txt
echo "Comparing with correct.txt"
diff out.txt correct.txt
