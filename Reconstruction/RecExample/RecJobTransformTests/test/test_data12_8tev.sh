#!/bin/sh
#
# art-description: Reco_tf runs on 2012 8 TeV collision data with all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py  --CA --multithreaded --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1 --maxEvents 300  --conditionsTag="COMCOND-BLKPA-RUN1-07" --geometryVersion="ATLAS-R1-2012-03-02-00" --outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root 

RES=$?
echo "art-result: $RES Reco"
