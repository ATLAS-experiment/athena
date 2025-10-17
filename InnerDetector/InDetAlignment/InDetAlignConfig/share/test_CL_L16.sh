#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

## Test iteration on L16 with use of local constant derived by L11 iteration
## In case the local db does not exist for some reason,
## allow this test to run anyway wihtout any local db input

if [ -f Iter0/Solve/condition.db ]; then
    LOCALDATABASE="--localDatabase Iter0/Solve/condition.db"
else
    LOCALDATABASE=""
fi

IDAlign_tf.py \
    --alignLevel 16 \
    --maxEvents ${MAXEVENTS} \
    --accumulate \
    --baseDir Iter1 \
    --logLevel ${LOGLEVEL} \
    --outputMonitorFile monitor.root \
    --outputTFile matrix.root \
    --execOnly \
    ${LOCALDATABASE}

IDAlign_tf.py \
    --alignLevel 16 \
    --solve \
    --logLevel ${LOGLEVEL} \
    --baseDir Iter1 \
    --inputTFile Iter1/Accumulate/matrix.root \
    --outputConditionFile condition_pool.root \
    --outputDBFile condition.db \
    --outputTaredLogFile align_logs.tar.gz \
    --execOnly \
    ${LOCALDATABASE}
