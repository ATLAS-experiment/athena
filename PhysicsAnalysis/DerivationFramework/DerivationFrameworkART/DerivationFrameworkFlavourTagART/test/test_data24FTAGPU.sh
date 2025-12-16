#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGPU data24
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data24/AOD/data24_13p6TeV.00486658.physics_Main.recon.AOD.f1522_m2262_r16385_r16377/AOD.43718985._000221.pool.root.1 \
--outputDAODFile art.pool.root \
--formats FTAGPU \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_FTAGPU.art.pool.root > checkFile_FTAGPU.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAGPU.art.pool.root > checkxAOD_FTAGPU.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAGPU.art.pool.root > checkIndexRefs_FTAGPU.txt 2>&1

echo "art-result: $?  checkIndexRefs"
