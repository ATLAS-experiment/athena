#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION12 mc23PbPb
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/AOD.47311601._002261.pool.root.1 \
--outputDAODFile art.pool.root \
--formats HION12 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HION12.art.pool.root > checkFile_HION12.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION12.art.pool.root > checkxAOD_HION12.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION12.art.pool.root > checkIndexRefs_HION12.txt 2>&1

echo "art-result: $?  checkIndexRefs"
