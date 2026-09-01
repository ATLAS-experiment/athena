#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HIGG1D1 data18
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN2_DATA[0])")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--formats HIGG1D1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HIGG1D1.art.pool.root > checkFile_HIGG1D1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HIGG1D1.art.pool.root > checkxAOD_HIGG1D1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HIGG1D1.art.pool.root > checkIndexRefs_HIGG1D1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
