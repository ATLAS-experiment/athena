#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION2 data25pO
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/data25_hip.00501607.physics_MinBias.AOD.f1604_m2272_skim \
--outputDAODFile art.pool.root \
--formats HION2 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HION2.art.pool.root > checkFile_HION2.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION2.art.pool.root > checkxAOD_HION2.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION2.art.pool.root > checkIndexRefs_HION2.txt 2>&1

echo "art-result: $?  checkIndexRefs"
