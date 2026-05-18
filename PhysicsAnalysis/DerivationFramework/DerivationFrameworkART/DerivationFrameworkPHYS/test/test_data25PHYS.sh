#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS data25
# art-type: grid
# art-input: user.martindl.data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232
# art-input-nfiles: 1
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

if [[ -z ${ArtInFile} ]]; then
    ArtInFile="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/data25/AOD/data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232/AOD.49752827._000024.pool.root.1"
fi

Derivation_tf.py \
--inputAODFile ${ArtInFile} \
--outputDAODFile art.pool.root \
--formats PHYS \
--maxEvents 1000 \

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile_PHYS.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYS.art.pool.root > checkxAOD_PHYS.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYS.art.pool.root > checkIndexRefs_PHYS.txt 2>&1

echo "art-result: $?  checkIndexRefs"
