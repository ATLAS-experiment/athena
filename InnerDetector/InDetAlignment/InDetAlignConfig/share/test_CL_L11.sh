#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

## Test iteration on L11
runIDAlign.py \
    --alignLevel 11 \
    --maxEvents ${MAXEVENTS} \
    --accumulate \
    --baseDir Iter0 \
    --logLevel ${LOGLEVEL} \
    --monitorFile monitor.root

runIDAlign.py \
    --alignLevel 11 \
    --solve \
    --logLevel ${LOGLEVEL} \
    --baseDir Iter0
