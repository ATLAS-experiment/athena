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
# art-output: RDO.pool.root

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Reco_tf.py \
--AMI=q454 \
--preExec "r2a:flags.DQ.Steering.HLT.doInDet=False; flags.Exec.FPE=500;" \
--postExec "" \
--multithreaded \
--maxEvents=500 \
--outputRDOFile=RDO.pool.root --outputAODFile=AOD.pool.root --outputESDFile=ESD.pool.root --outputHISTFile=myHIST.root \
--conditionsTag "all:${conditions}" \
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
