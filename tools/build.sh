#!/bin/sh

INCLUDE_DIR=/home/wedge/01pol/include/
LIB_DIR=/home/wedge/01pol/libpolytools/

#gcc -O2 -Wall -o incidence incidence.c lrs/lrslib.c lrs/lrsmp.c
gcc -O2 -Wall -o dupe dupe.c
mpicc -O2 -Wl,--rpath,$LIB_DIR -L$LIB_DIR -I$INCLUDE_DIR -Wall -lz -lpolytools -o facetcnt facetcnt.c 
#mpicc -O2 -Wall -o find_pol1 find_pol1.c lrs/lrslib.c lrs/lrsmp.c
