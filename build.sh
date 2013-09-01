#!/bin/bash

INCLUDE_DIR=/home/wedge/01pol/include/
LIB_DIR=/home/wedge/01pol/libpolytools/

mpic++ -Wl,--rpath,$LIB_DIR -L$LIB_DIR -I$INCLUDE_DIR -lpolytools increase_new.cpp -o increase_new
