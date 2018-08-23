#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=./resources/normal_file.c
EXPECT="Found 1 files, 2 lines"

result=$($NGP $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi
