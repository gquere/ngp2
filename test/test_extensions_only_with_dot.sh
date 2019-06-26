#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=./resources/extensions/

EXPECT="Found 1 files, 1 lines"
result=$($NGP $PATTERN $RESOURCE -o '.bla')

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
