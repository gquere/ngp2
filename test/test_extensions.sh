#!/bin/bash

NGP=../ngp_perf
PATTERN="test"
RESOURCE=./resources/extensions/

EXPECT="Found 0 files, 0 lines"
result=$($NGP $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

EXPECT="Found 2 files, 2 lines"
result=$($NGP $PATTERN $RESOURCE -t 'a')

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
