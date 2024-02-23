#!/bin/sh
#
# art-description: Reco_tf runs on 2011 7 TeV collision data with all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py  --CA --multithreaded --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/high_mu-data11_7TeV.00179725.physics_JetTauEtmiss.merge.RAW._lb0021.data --maxEvents 300  --conditionsTag="COMCOND-BLKPA-RUN1-07"  --geometryVersion="ATLAS-R1-2011-02-00-00" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root 

RES=$?
echo "art-result: $RES Reco"
