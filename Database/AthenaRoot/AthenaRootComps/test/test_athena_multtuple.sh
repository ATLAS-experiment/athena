#!/bin/bash

set -e

testdir=`arc_linkinputs.sh`
arc_test_make_slim.py $testdir/ntuple.0.root ntuple_slim.0.root 100
arc_test_make_slim.py $testdir/ntuple.1.root ntuple_slim.1.root 100

echo "::::::::::::::::::::::::::::::::::::::::::::::"
echo "::: run athena-ntuple-dumper... (w/ multiple tuples)"
test_athena_ntuple_dumper.py --filesInput "ntuple_slim.0.root,ntuple_slim.1.root" --tupleName "egamma;egamma_der" -l ERROR >& d3pd.000.0.log.txt
