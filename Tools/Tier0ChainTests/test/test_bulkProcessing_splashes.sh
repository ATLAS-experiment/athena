#!/bin/sh
#
# art-description: Express processing at Tier0
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

# TODO update following ATLASRECTS-8054

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")

Reco_tf.py  \
--AMI f1328  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_comm.00420624.express_express.merge.RAW._lb1054._SFO-ALL._0001.1" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputHISTFile="HIST.root" \
--conditionsTag="${conditions}" \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 30 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} (against previous nightly)"
