#!/bin/sh

MAXEVENTS=$1
LOGLEVEL=$2

[ ! -n "${MAXEVENTS}" ] && MAXEVENTS=10
[ ! -n "${LOGLEVEL}" ] && LOGLEVEL=INFO

INPUT_FILE=$(python -c \
"from AthenaConfiguration.TestDefaults import defaultTestFiles; \
print(defaultTestFiles.RDO_RUN4[0])")

## Test iteration on L11
IDAlign_tf.py \
    --alignLevel 3 \
    --maxEvents ${MAXEVENTS} \
    --inputRAWFile ${INPUT_FILE} \
    --accumulate \
    --alignITk \
    --alignITkPixel \
    --inputTracksCollection CombinedITkTracks \
    --atlasVersion ATLAS-P2-RUN4-04-00-00 \
    --globalTag OFLCOND-MC21-SDR-RUN4-05 \
    --baseDir Iter0 \
    --logLevel ${LOGLEVEL} \
    --outputMonitorFile monitor.root \
    --execOnly \
    --outputTFile matrix.root

IDAlign_tf.py \
    --alignLevel 3 \
    --solve \
    --logLevel ${LOGLEVEL} \
    --baseDir Iter0 \
    --inputTFile Iter0/Accumulate/matrix.root \
    --outputConditionFile condition_pool.root \
    --outputDBFile condition.db \
    --execOnly \
    --outputTaredLogFile align_logs.tar.gz
 


 #  --globalTag OFLCOND-MC21-SDR-RUN4-05 \
