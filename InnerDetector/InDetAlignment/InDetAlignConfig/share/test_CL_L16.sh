#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

## Test iteration on L16 with use of local constant derived by L11 iteration
runIDAlign.py \
    --alignLevel 16 \
    --maxEvents ${MAXEVENTS} \
    --accumulate \
    --baseDir Iter1 \
    --monitorFile monitor.root \
    --logLevel ${LOGLEVEL} \
    --localDatabase Iter0/Solve/alignment_output.db

runIDAlign.py \
    --alignLevel 16 \
    --solve \
    --baseDir Iter1 \
    --logLevel ${LOGLEVEL} \
    --localDatabase Iter0/Solve/alignment_output.db
