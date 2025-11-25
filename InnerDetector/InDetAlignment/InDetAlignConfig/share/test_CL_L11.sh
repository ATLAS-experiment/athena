#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

## Test iteration on L11
IDAlign_tf.py \
    --alignLevel 11 \
    --maxEvents ${MAXEVENTS} \
    --accumulate \
    --baseDir Iter0 \
    --logLevel ${LOGLEVEL} \
    --outputMonitorFile monitor.root \
    --execOnly \
    --outputTFile matrix.root

IDAlign_tf.py \
    --alignLevel 11 \
    --solve \
    --logLevel ${LOGLEVEL} \
    --baseDir Iter0 \
    --inputTFile Iter0/Accumulate/matrix.root \
    --outputConditionFile condition_pool.root \
    --outputDBFile condition.db \
    --execOnly \
    --outputTaredLogFile align_logs.tar.gz
