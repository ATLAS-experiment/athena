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
# art-output: AOD.pool.root
# art-output: ESD.pool.root

Reco_tf.py \
--CA "True" \
--AMI=q442 \
--conditionsTag 'all:CONDBR2-BLKPA-RUN2-11' \
--athenaopts='--nprocs=2' \
--maxEvents=500 \
--outputAODFile=AOD.pool.root --outputESDFile=ESD.pool.root \
--imf False \
--preExec "all:flags.DQ.Steering.doHLTMon=False" 

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ $rc1 -eq 0 ]
then
  art.py compare grid --entries 20 "$1" "$2" --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo "art-result: $rc2 Diff"
