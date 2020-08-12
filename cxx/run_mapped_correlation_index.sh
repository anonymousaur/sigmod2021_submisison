#!/bin/bash

ARGPREFIX=$1
WORKLOAD=$2

./build2/run_mapped_correlation_index \
    --dataset=$ARGPREFIX.data \
    --workload=$WORKLOAD \
    --visitor=index \
    --mapping-file=$ARGPREFIX.mapping \
    --outlier-list=$ARGPREFIX.outliers \
    --target-bucket-file=$ARGPREFIX.targets

