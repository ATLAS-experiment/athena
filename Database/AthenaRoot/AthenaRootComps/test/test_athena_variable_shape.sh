#!/bin/bash

set -e

branches=$1

echo "::: generate d3pd.root..."
test_athena_variable_shape_ntuple.py \
    --evtMax 10 --branches "RunNumber,EventNumber,${branches}" --outbranches "${branches}" \
    &> log.txt

(acmd.py dump-root d3pd.root 2> d3pd.stderr.ascii) \
    | grep "^egamma" \
    | egrep "RunNumber|EventNumber|el_n" \
    | tee d3pd.ascii

cat d3pd.ascii| cut -d. -f3 >| d3pd.ascii.todiff
