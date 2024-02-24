#!/bin/sh
#
# art-description: Reco_tf runs on 2022 13.6 TeV collision data for all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

#Based on https://twiki.cern.ch/twiki/bin/view/Atlas/Quick2022Run3Reprocessing
#In CA all the monitoring flags are true by default, so don't set them.
#Don't touch ZDC flag, which is off by default.

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA --multithreaded  --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data --maxEvents 300 --conditionsTag="CONDBR2-BLKPA-2022-09" --geometryVersion="ATLAS-R3S-2021-03-01-00" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

#Remember retval of transform as art result
RES=$?
echo "art-result: $RES Reco"

