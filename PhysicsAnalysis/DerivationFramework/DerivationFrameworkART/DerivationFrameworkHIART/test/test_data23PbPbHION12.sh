#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION12 data23PbPb
# art-input: data23_hi:data23_hi.00463364.physics_UPC.merge.AOD.r16069_p6447
# art-input-nfiles: 1
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile ${ArtInFile} \
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
