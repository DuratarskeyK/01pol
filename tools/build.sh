#!/bin/sh

mpicc -O2 -Wall -o facetcnt facetcnt.c lrs/lrslib.c lrs/lrsmp.c
mpicc -O2 -Wall -o find_pol1 find_pol1.c lrs/lrslib.c lrs/lrsmp.c
