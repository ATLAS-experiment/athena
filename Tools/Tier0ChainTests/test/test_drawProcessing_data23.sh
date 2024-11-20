#!/bin/sh
#
# art-description: Reco_tf processing of DRAW_EGZ and DRAW_ZMUMU inputs
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

Reco_tf.py \
--AMI f1350  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/DRAW_EGZ/data23_13p6TeV.00456714.physics_Main.merge.DRAW_EGZ.f1370_m2193/312events.data23_13p6TeV.00456714.physics_Main.merge.DRAW_EGZ.f1370_m2193._0602.1" \
--outputDESDM_ALLCELLSFile="myDESDM_EGZ.pool.root" \
--outputDAOD_L1CALO1File="myDAOD_L1CALO1EGZ.pool.root" \
--maxEvents 75 \
--conditionsTag="CONDBR2-BLKPA-2022-15" \
--imf False
rc1=$?
echo "art-result: $rc1 Reco DRAW_EGZ"

Reco_tf.py \
--AMI f1350  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/DRAW_ZMUMU/data23_13p6TeV.00456386.physics_Main.merge.DRAW_ZMUMU.f1369_m2193/540events.data23_13p6TeV.00456386.physics_Main.merge.DRAW_ZMUMU.f1369_m2193._0042.1" \
--outputAODFile="myDAOD_ZMUMU.pool.root" \
--outputDAOD_L1CALO1File="myDAOD_L1CALO1ZMM.pool.root" \
--maxEvents 75 \
--conditionsTag="CONDBR2-BLKPA-2022-15" \
--imf False
rc2=$?
echo "art-result: $rc2 Reco DRAW_ZMUMU"

rc3=-9999
if [ ${rc1} -eq 0 ] || [ ${rc2} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 25 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc3=$?
fi
echo  "art-result: ${rc3} (against previous nightly)"
