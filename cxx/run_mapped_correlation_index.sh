#!/bin/bash

NAME=$1
ARGPREFIX=$2
WORKLOAD=$3

if [ -z "$WORKLOAD" ];
then
    echo "Requires 3 arguments"
    exit 0
fi

./build2/run_mapped_correlation_index \
    --name=$NAME \
    --dataset=$ARGPREFIX.data \
    --workload=$WORKLOAD \
    --visitor=sum \
    --mapping-file=$ARGPREFIX.mapping \
    --outlier-list=$ARGPREFIX.outliers \
    --target-bucket-file=$ARGPREFIX.targets \
    --save=results/$NAME.dat

