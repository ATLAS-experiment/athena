#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TLA2 mc21_14TeV_
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN4_MC[0])")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--formats TLA2 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_TLA2.art.pool.root > checkFile_TLA2.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TLA2.art.pool.root > checkxAOD_TLA2.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TLA2.art.pool.root > checkIndexRefs_TLA2.txt 2>&1

echo "art-result: $?  checkIndexRefs"
