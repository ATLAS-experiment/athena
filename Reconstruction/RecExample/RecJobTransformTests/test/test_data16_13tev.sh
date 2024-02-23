#!/bin/sh
#
# art-description: Reco_tf runs on 2016 13 TeV collision data with all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
#The setup for run2 data is described in test_data15_13TeV.sh
Reco_tf.py  --CA --multithreaded --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data16_13TeV.00310809.physics_Main.daq.RAW._lb1219._SFO-2._0001.data --maxEvents 300 --conditionsTag="CONDBR2-BLKPA-RUN2-11" --geometryVersion="ATLAS-R2-2016-01-00-01" --preExec="flags.DQ.Steering.doHLTMon=False" --ignorePatterns 'LArRawDataReadingAlg.+ERROR.+Found.+unsupported.+Rod.+block.+type.+0|LArRawDataReadingAlg.+\|.+ERROR.+\|.|ERROR.+message.+limit.+LArRawDataReadingAlg.'  --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"
