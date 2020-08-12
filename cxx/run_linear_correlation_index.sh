#!/bin/bash

ARGPREFIX=$1
WORKLOAD=$2

./build2/run_linear_correlation_index \
    --dataset=$ARGPREFIX.data \
    --workload=$WORKLOAD \
    --visitor=index \
    --rewriter=$ARGPREFIX.model \
    --outlier-list=$ARGPREFIX.outliers

