#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM5 mc23
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC[0])")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
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
