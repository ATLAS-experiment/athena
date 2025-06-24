#!/bin/bash
# art-description: new DQ in Reco_tf, data22 bulk
# art-type: grid
# art-memory: 6144
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: myHIST.root
# art-output: log*
# art-athena-mt: 3

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")
Reco_tf.py \
--athenaopts='--threads=1' \
--AMI=f1328 \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00437548.physics_Main.daq.RAW._lb1044._SFO-15._0002.data" \
--maxEvents=25 \
--outputAODFile=AOD.pool.root \
--outputHISTFile=myHIST.root \
--conditionsTag=$conditionsTag \
--imf False

echo "art-result: $? HIST_Creation"
rm -rf ref-*

ArtPackage=$1
ArtJobName=$2
art.py download ${ArtPackage} ${ArtJobName}
REFFILE=(./ref-*/myHIST.root)
hist_diff.sh myHIST.root $REFFILE -x 'TIME_(execute|convert|prepareROBs)' -i > log.HIST_Diff 2>&1
echo "art-result: $? HIST_Diff"
