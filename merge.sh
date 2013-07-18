#!/usr/bin/bash

t=1

while [ `ls tmp | wc -l` != '1' ]
do
	state=0
	a=0
	t=$(($t+1))
	for i in `ls tmp` ; do
	    if [[ $state -eq 0 ]] ; then
        	state=1
	        save=$i
	    else
        	state=0
		./merge $1 tmp/$i tmp/$save tmp/$t$a
		rm tmp/$i tmp/$save
		a=$(($a+1))
	    fi
	done
done
