#!/bin/sh
#
# art-description: Reco_tf runs on 2022 13.6 TeV collision data with FPE crashes enabled.
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8

#Settings same as test_data22_13p6TeV.sh
Reco_tf.py --CA --multithreaded  --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data --maxEvents 300 --conditionsTag="CONDBR2-BLKPA-2022-09" --geometryVersion="ATLAS-R3S-2021-03-01-00" --preExec="flags.Exec.FPE=-1" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"

