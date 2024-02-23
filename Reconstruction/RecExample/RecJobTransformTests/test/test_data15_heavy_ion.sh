#!/bin/sh
#
# art-description: Reco_tf runs on 2015 Heavy Ion data with MinBiasOverlay stream (contact Andrzej Olszewski and Iwona Grabowska-Bold)
# art-athena-mt: 4
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
--CA \
--multithreaded \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data15_hi.00286711.physics_MinBiasOverlay.daq.RAW._lb0217._SFO-2._0001.data \
--geometryVersion="ATLAS-R2-2015-03-01-00" \
--conditionsTag="CONDBR2-BLKPA-RUN2-11" \
--autoConfiguration 'everything' \
--preInclude="all:HIRecConfig.HIModeFlags.HImode" \
--preExec="flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False" \
--outputAODFile=myAOD.pool.root \
--maxEvents=25

RES=$?
echo "art-result: $RES Reco"
