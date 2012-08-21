#!/bin/sh

if [ $# -gt 0 ]; then
	if [ $1 = remove ]; then
		rm -f *.so *.o
		exit 0
	fi
fi

gcc -fPIC -c tycoonclient.c 

if [ $? -ne 0 ]; then
	exit 1;
else
	gcc -shared -o libtycoonclient.so tycoonclient.o && echo "Library builded."
fi

