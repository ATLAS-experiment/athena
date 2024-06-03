#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building NCB1 data23cos
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23_cos/AOD/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195/data23_cos.00459152.physics_CosmicMuons.merge.AOD.f1383_m2195._lb0124-lb0126._0001.1 \
--outputDAODFile art.pool.root \
--formats NCB1 \
--maxEvents 2000 \

echo "art-result: $? reco"

checkFile.py DAOD_NCB1.art.pool.root > checkFile_NCB1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_NCB1.art.pool.root > checkxAOD_NCB1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_NCB1.art.pool.root > checkIndexRefs_NCB1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
