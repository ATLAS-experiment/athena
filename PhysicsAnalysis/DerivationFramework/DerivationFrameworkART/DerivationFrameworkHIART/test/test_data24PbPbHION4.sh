#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION4 data24PbPb
# art-input: data24_hi:data24_hi.00490145.physics_UPC.merge.AOD.f1550_m2267
# art-input-nfiles: 1
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile ${ArtInFile} \
--outputDAODFile art.pool.root \
--formats HION4 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HION4.art.pool.root > checkFile_HION4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION4.art.pool.root > checkxAOD_HION4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION4.art.pool.root > checkIndexRefs_HION4.txt 2>&1

echo "art-result: $?  checkIndexRefs"
