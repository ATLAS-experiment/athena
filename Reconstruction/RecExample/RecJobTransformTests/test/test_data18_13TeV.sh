#!/bin/sh
#
# art-description: Reco_tf runs on 2018 13 TeV collision data with all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA --multithreaded  --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data18_13TeV/data18_13TeV.00348885.physics_Main.daq.RAW._lb0827._SFO-8._0002.data --maxEvents 300 --conditionsTag="CONDBR2-BLKPA-RUN2-11" --geometryVersion="ATLAS-R2-2016-01-00-01" --preExec="flags.DQ.Steering.doHLTMon=False" --ignoreErrors 'False' --ignorePatterns 'LArRawDataReadingAlg.+ERROR.+Found.+unsupported.+Rod.+block.+type.+0|LArRawDataReadingAlg.+\|.+ERROR.+\|.|ERROR.+message.+limit.+LArRawDataReadingAlg.' --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"

