#!/bin/bash

curdir=`pwd`

cd punchline/stocks
./run_exp_btree.sh
./run_exp_octree.sh
cd $curdir

echo "===================="
echo "===================="
echo

cd punchline/chicago_taxi
./run_exps_btree.sh
./run_exps_octree.sh

cd $curdir

