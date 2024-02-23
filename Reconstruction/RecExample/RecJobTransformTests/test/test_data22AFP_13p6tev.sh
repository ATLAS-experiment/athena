#!/bin/sh
#
# art-description: Reco_tf runs on 2022 13.6TeV collision data, where Run 435229 includes AFP detector. Details at https://twiki.cern.ch/twiki/bin/view/AtlasProtected/SpecialRunsIn2022#done_47_SM_HI_combined_LHCf_ZDC
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
#Settings same as test_data22_13p6TeV.sh
Reco_tf.py  --CA --multithreaded  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00435229.physics_Main.daq.RAW/data22_13p6TeV.00435229.physics_Main.daq.RAW._lb1526._SFO-12._0001.data --conditionsTag="CONDBR2-BLKPA-2022-09" --geometryVersion="ATLAS-R3S-2021-03-01-00" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root --maxEvents=10

#Remember retval of transform as art result
RES=$?
xAODDigest.py myAOD.pool.root digest.txt
echo "art-result: $RES Reco"

