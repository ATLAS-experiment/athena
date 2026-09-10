#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building BPHY6 mc21_14TeV_
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN4_MC[0])")

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats BPHY6 \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_BPHY6.art.pool.root > checkFile_BPHY6.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY6.art.pool.root > checkxAOD_BPHY6.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_BPHY6.art.pool.root > checkIndexRefs_BPHY6.txt 2>&1

echo "art-result: $?  checkIndexRefs"
