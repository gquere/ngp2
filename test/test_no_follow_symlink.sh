#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=./resources/file_symlink.c
EXPECT="Found 0 files, 0 lines"

result=$($NGP $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
