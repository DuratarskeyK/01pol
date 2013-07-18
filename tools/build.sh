#!/bin/sh

gcc -O2 -Wall -o incidence incidence.c lrs/lrslib.c lrs/lrsmp.c
gcc -O2 -Wall -o dupe dupe.c
mpicc -O2 -Wall -o facetcnt facetcnt.c lrs/lrslib.c lrs/lrsmp.c
mpicc -O2 -Wall -o find_pol1 find_pol1.c lrs/lrslib.c lrs/lrsmp.c
