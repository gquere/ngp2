#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=./resources/file_with_no_ending_newline.c
EXPECT="Found 1 files, 1 lines"

result=$($NGP $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
