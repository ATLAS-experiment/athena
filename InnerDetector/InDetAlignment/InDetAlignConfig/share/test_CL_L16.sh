#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

## Test iteration on L16 with use of local constant derived by L11 iteration
## In case the local db does not exist for some reason,
## allow this test to run anyway wihtout any local db input

if [ -f Iter0/Solve/alignment_output.db ]; then
    LOCALDATABASE="--localDatabase Iter0/Solve/alignment_output.db"
else
    LOCALDATABASE=""
fi

runIDAlign.py \
    --alignLevel 16 \
    --maxEvents ${MAXEVENTS} \
    --accumulate \
    --baseDir Iter1 \
    --monitorFile monitor.root \
    --logLevel ${LOGLEVEL} \
    ${LOCALDATABASE}

runIDAlign.py \
    --alignLevel 16 \
    --solve \
    --baseDir Iter1 \
    --logLevel ${LOGLEVEL} \
    ${LOCALDATABASE}
