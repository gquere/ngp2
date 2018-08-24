#!/bin/bash

NGP=../ngp_perf
PATTERN="int"
RESOURCE=./resources/file_with_ignored_extension.txt
EXPECT="Found 1 files, 2 lines"

result=$($NGP -r $PATTERN $RESOURCE)

if [ "$result" != "$EXPECT" ]
then
    echo "$0 failed"
    echo "Expected: '$EXPECT'"
    echo "Got: '$result'"
    exit -1
fi

echo "$0 OK"
