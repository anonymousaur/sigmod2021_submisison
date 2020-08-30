#!/bin/bash

DIM=$1
NAME=$2
DATASET=$3
WORKLOAD=$4
SPEC=$5

if [ -z "$SPEC" ];
then
    echo "Requires 5 arguments"
    exit 0
fi

./build${DIM}/run_mapped_correlation_index \
    --name=$NAME \
    --dataset=$DATASET \
    --workload=$WORKLOAD \
    --visitor=index \
    --indexer-spec=$SPEC \
    --save=results/$NAME.dat

