#!/bin/sh
#
# art-description: merging check with Tier0 setup
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-output: myAOD.pool.root
# art-output: myDAOD_IDTIDE.pool.root

aod=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1
Merge_tf.py \
--CA True \
--autoConfiguration=everything \
--inputAODFile="${aod},${aod}" \
--outputAOD_MRGFile=myAOD.pool.root
rc1=$?
echo "art-result: $rc1 for AOD merging"

idtide=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/DAOD_IDTIDE/data23_13p6TeV.00453713.physics_Main.recon.DAOD_IDTIDE.f1357/21events.data23_13p6TeV.00453713.physics_Main.recon.DAOD_IDTIDE.f1357._lb1070._0007.1
Merge_tf.py \
--CA True \
--autoConfiguration=everything \
--inputAODFile="${idtide},${idtide}" \
--outputAOD_MRGFile=myDAOD_IDTIDE.pool.root
rc2=$?
echo "art-result: $rc2 for DAOD_IDTIDE merging"

mcp=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/DESDM_MCP/data23_13p6TeV.00453713.physics_Main.recon.DESDM_MCP.f1357/759events.data23_13p6TeV.00453713.physics_Main.recon.DESDM_MCP.f1357._lb1406._0007.1
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")
Merge_tf.py \
--CA True \
--autoConfiguration=everything \
--conditionsTag=${CONDTAG} \
--inputESDFile="${mcp},${mcp}" \
--outputESD_MRGFile=myDESDM_MCP.pool.root
rc3=$?
echo "art-result: $rc3 for DESDM_MCP merging"

rc4=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 5 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc4=$?
fi
echo  "art-result: ${rc4} (comparison against previous nightly)"
