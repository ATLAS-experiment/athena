#!/bin/bash

set -e

reffile=$1

echo "::: generate merged.root..."
test_athena_variable_shape_ntuple.py \
    --evtMax 30 --branches "RunNumber,EventNumber,el_n,el_eta,el_phi" --outbranches "el_n" \
    --filesInput="../unitTestRun_test_athena_variable_shape1/d3pd.root,../unitTestRun_test_athena_variable_shape2/d3pd.root,../unitTestRun_test_athena_variable_shape3/d3pd.root" \
    &> log.txt

(acmd.py dump-root d3pd.root 2> d3pd.stderr.txt) \
    | grep "^egamma" \
    | egrep "RunNumber|EventNumber|el_n" \
    | tee d3pd.ascii
cat d3pd.ascii| cut -d. -f3 >| d3pd.ascii.todiff

echo "::: compare py-alg outputs..."
cat ../unitTestRun_test_athena_variable_shape[123]/data.var.txt > data.merged.txt
diff -urN data.merged.txt data.var.txt
echo "::: compare py-alg outputs... [ok]"

echo "::: compare py-alg output to reference..."
diff -urN data.merged.txt $reffile
echo "::: compare py-alg output to reference... [ok]"

echo "::: compare dump-root outputs..."
cat ../unitTestRun_test_athena_variable_shape[123]/d3pd.ascii.todiff > merged.ascii.todiff
diff -urN merged.ascii.todiff d3pd.ascii.todiff
echo "::: compare dump-root outputs... [ok]"
