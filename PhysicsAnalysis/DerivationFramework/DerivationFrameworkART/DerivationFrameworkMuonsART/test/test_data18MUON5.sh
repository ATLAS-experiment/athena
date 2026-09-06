#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building MUON5 data18
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
--formats MUON5 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_MUON5.art.pool.root > checkFile_MUON5.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_MUON5.art.pool.root > checkxAOD_MUON5.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_MUON5.art.pool.root > checkIndexRefs_MUON5.txt 2>&1

echo "art-result: $?  checkIndexRefs"
