#!/bin/sh

TYCOON_LIB="libtycoon.so"
TYCOON_HEADER="tycoonclient.h"
LIB_PATH=/usr/lib
HEADER_PATH=/usr/include

if [ $# -gt 0 ]; then
	if [ $1 = remove ]; then
		rm -f $LIB_PATH/$TYCOON_LIB $HEADER_PATH/$TYCOON_HEADER
		exit 0
	fi
fi


if [ -f $LIB_PATH/$TYCOON_LIB]; then
	rm -f $LIB_PATH/$TYCOON_LIB
	cp $TYCOON_LIB $LIB_PATH/$TYCOON_LIB
else
	cp $TYCOON_LIB $LIB_PATH/$TYCOON_LIB
fi

if [ -f $HEADER_PATH/$TYCOON_HEADER]; then
	rm -f $HEADER_PATH/$TYCOON_HEADER
	cp $TYCOON_HEADER $HEADER_PATH/$TYCOON_HEADER
else
	cp $TYCOON_HEADER $HEADER_PATH/$TYCOON_HEADER
fi

