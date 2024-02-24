#!/bin/sh
#
# art-description: Reco_tf runs on 2018 Heavy Ion data with HardProbes stream (contact Sebastian Tapia)
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
export TRF_ECHO=True; 

Reco_tf.py \
--CA \
--multithreaded \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data \
--steering='doRAWtoALL' \
--geometryVersion 'ATLAS-R2-2016-01-00-01' \
--conditionsTag 'CONDBR2-BLKPA-RUN2-11' \
--preInclude="all:HIRecConfig.HIModeFlags.HImode" \
--preExec="flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False" \
--outputAODFile=AOD.pool.root \
--maxEvents=20

RES=$?
echo "art-result: $RES Reco"
