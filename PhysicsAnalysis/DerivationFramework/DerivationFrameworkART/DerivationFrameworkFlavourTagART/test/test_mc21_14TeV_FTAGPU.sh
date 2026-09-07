#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGPU mc21_14TeV_
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
--formats FTAGPU \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_FTAGPU.art.pool.root > checkFile_FTAGPU.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAGPU.art.pool.root > checkxAOD_FTAGPU.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAGPU.art.pool.root > checkIndexRefs_FTAGPU.txt 2>&1

echo "art-result: $?  checkIndexRefs"
