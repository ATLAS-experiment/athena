#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM7 data18
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN2_DATA[0])")

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats EGAM7 \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_EGAM7.art.pool.root > checkFile_EGAM7.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM7.art.pool.root > checkxAOD_EGAM7.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM7.art.pool.root > checkIndexRefs_EGAM7.txt 2>&1

echo "art-result: $?  checkIndexRefs"
