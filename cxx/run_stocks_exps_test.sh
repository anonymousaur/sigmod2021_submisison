#!/bin/bash
set -x

BASE_DIR=/home/ubuntu/correlations/continuous/stocks

./run_mapped_correlation_index.sh 7 stocks_corrix_a1_p1000_q3_s05_p1000 \
    $BASE_DIR/stocks_data_sort_octree_0_4_p1000.bin \
    $BASE_DIR/queries_3_s05.dat \
    $BASE_DIR/index_0_4_p1000_a1.build > results/raw_stocks_0_4_p1000_s05.out

./run_mapped_correlation_index.sh 7 stocks_octree_p1000_q3_s05 \
    $BASE_DIR/stocks_data_sort_octree_0_4_p1000.bin \
    $BASE_DIR/queries_3_s05.dat \
    $BASE_DIR/index_octree_p1000.build > results/raw_stocks_octree_p1000_s05.out

./run_mapped_correlation_index.sh 7 stocks_secondary_p1000_q3_s05 \
    $BASE_DIR/stocks_data_sort_octree_0_4_p1000.bin \
    $BASE_DIR/queries_3_s05.dat \
    $BASE_DIR/index_secondary_p1000.build > results/raw_stocks_secondary_p1000_s05.out 

