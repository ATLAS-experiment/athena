#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM5 data18
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1 \
--outputDAODFile art.pool.root \
--formats EGAM5 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_EGAM5.art.pool.root > checkFile_EGAM5.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM5.art.pool.root > checkxAOD_EGAM5.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM5.art.pool.root > checkIndexRefs_EGAM5.txt 2>&1

echo "art-result: $?  checkIndexRefs"
