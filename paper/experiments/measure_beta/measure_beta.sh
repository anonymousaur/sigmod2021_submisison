#!/bin/bash

BINARY_DIR=/home/ubuntu/correlations/cxx
EXP_DIR=/home/ubuntu/correlations/paper/experiments/measure_beta
queries=$EXP_DIR/queries_d2.dat
function join_by { local IFS="$1"; shift; echo "$*"; }

all_queries=`join_by , "${queries[@]}"`
echo "Querying workloads: $all_queries"

binary=$EXP_DIR/gaussian_noise_f10.bin

$BINARY_DIR/run_mapped_correlation_index.sh 2 measure_beta \
    $binary \
    $queries \
    $EXP_DIR/indexes/index_measure_beta.build

