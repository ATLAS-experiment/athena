#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGXBB data22
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_DATA[0])")

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats FTAGXBB \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_FTAGXBB.art.pool.root > checkFile_FTAGXBB.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAGXBB.art.pool.root > checkxAOD_FTAGXBB.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAGXBB.art.pool.root > checkIndexRefs_FTAGXBB.txt 2>&1

echo "art-result: $?  checkIndexRefs"
