#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8                                                                                                                                     

Reco_tf.py \
--AMI=q442 \
--conditionsTag 'all:CONDBR2-BLKPA-RUN2-11' \
--CA "True" \
--preExec "all:flags.DQ.Steering.doHLTMon=False" \
--athenaopts='--threads=8' \
--maxEvents=500 \
--outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --outputHISTFile=myHIST.root \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ $rc1 -eq 0 ]
then
  art.py compare grid --entries 50 "$1" "$2" --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo "art-result: $rc2 Diff"
