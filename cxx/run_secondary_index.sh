#!/bin/bash

NAME=$1
ARGPREFIX=$2
WORKLOAD=$3

if [ -z "$WORKLOAD" ];
then
    echo "Requires 3 arguments"
    exit 0
fi

./build2/run_secondary_index \
    --name=$NAME \
    --dataset=$ARGPREFIX.data \
    --workload=$WORKLOAD \
    --visitor=sum \
    --mapped-dim=0 \
    --target-dim=1 \
    --save=results/$NAME.dat

