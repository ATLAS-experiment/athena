#!/bin/sh
#
# art-description: Reco_tf runs on 2022 Heavy Ion data with Minbias stream (HI mode)
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
--CA \
--multithreaded \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_hi/RAWFiles/data22_hi.00440101.physics_MinBias.daq.RAW/data22_hi.00440101.physics_MinBias.daq.RAW._lb0214._SFO-11._0001.data \
--geometryVersion="ATLAS-R3S-2021-03-01-00" \
--conditionsTag="CONDBR2-BLKPA-2022-09" \
--preInclude="all:HIRecConfig.HIModeFlags.HImode" \
--preExec="flags.Egamma.doForward=False;" \
--outputAODFile=myAOD.pool.root \
--maxEvents=300

RES=$?
echo "art-result: $RES Reco"
