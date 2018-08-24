#!/bin/bash

NGP=../ngp_perf
PATTERN="in[t]"
RESOURCE=./resources/normal_file.c
EXPECT="Found 1 files, 2 lines"

result=$($NGP -e $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
