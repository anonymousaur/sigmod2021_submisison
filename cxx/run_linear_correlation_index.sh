#!/bin/bash

NAME=$1
ARGPREFIX=$2
WORKLOAD=$3

if [ -z "$WORKLOAD" ];
then
    echo "Requires 3 arguments"
    exit 0
fi

./build2/run_linear_correlation_index \
    --name=$NAME \
    --dataset=$ARGPREFIX.data \
    --workload=$WORKLOAD \
    --visitor=sum \
    --rewriter=$ARGPREFIX.model \
    --outlier-list=$ARGPREFIX.outliers \
    --save=results/$NAME.dat

