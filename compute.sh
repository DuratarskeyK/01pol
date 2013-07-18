#!/bin/bash

salloc -F host -n 112 mpirun -nooversubscribe -rf rankfile ./increase_new $1 $2 $3
mv buff* tmp/
sh merge.sh $(($2+1))
