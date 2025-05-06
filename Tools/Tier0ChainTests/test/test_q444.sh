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

Reco_tf.py \
--AMI=q444 \
--CA "Overlay,RAWtoALL:True" \
--preExec="RAWtoALL:flags.Exec.FPE=500" \
--postExec="" \
--conditionsTag 'all:OFLCOND-MC16-SDR-RUN2-12' \
--multithreaded \
--steering doOverlay doRDO_TRIG \
--maxEvents=500 \
--outputRDOFile=RDO.pool.root --outputAODFile=AOD.pool.root --outputESDFile=ESD.pool.root --outputHISTFile=myHIST.root \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ $rc1 -eq 0 ]
then
  art.py compare grid --entries 20 "$1" "$2" --mode=semi-detailed --order-trees --ignore-exit-code diff-pool \
  --ignore-leave 'Token' --ignore-leave 'index_ref' --ignore-leave '(.*)_timings\.(.*)' --ignore-leave '(.*)_mems\.(.*)' --ignore-leave '(.*)TrigCostContainer(.*)' --ignore-leave '(.*)HLTNav_Summary_OnlineSlimmed(.*)'
  rc2=$?
fi
echo "art-result: $rc2 Diff"
