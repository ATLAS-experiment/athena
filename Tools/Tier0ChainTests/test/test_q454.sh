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
--AMI=q454 \
--preExec "r2a:flags.DQ.Steering.HLT.doInDet=False; flags.Exec.FPE=500;" \
--postExec "" \
--multithreaded \
--maxEvents=500 \
--outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --outputHISTFile=myHIST.root \
--conditionsTag 'all:OFLCOND-MC23-SDR-RUN3-06' \
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
