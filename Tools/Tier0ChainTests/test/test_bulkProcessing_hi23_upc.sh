#!/bin/sh
#
# art-description: Bulk processing of the HeavyIon UPC-stream at the Tier0
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8
# art-output: AOD.pool.root
# art-output: ESD.pool.root

# TODO update following ATLASRECTS-8054

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")
Reco_tf.py  \
--AMI f1406 \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data23_hi.00462107.physics_UPC.daq.RAW._lb0500._SFO-14._0002.data" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputHISTFile="HIST.root" \
--maxEvents=1500 \
--conditionsTag=$conditionsTag \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Diff"

