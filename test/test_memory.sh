#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=/usr/src

if ! [ -x "$(command -v valgrind)" ]
then
    echo "Error: valgrind is not installed"
    exit -1
fi

echo "Running memory test ..."
echo "Warning: this could be long!"

result=$(valgrind --leak-check=full --error-exitcode=1 $NGP $PATTERN $RESOURCE)

if [ "$?" != 0 ]
then
    echo "$0 failed"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
