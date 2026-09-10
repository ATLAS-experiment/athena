#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TRIG9 data22
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
--formats TRIG9 \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_TRIG9.art.pool.root > checkFile_TRIG9.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRIG9.art.pool.root > checkxAOD_TRIG9.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRIG9.art.pool.root > checkIndexRefs_TRIG9.txt 2>&1

echo "art-result: $?  checkIndexRefs"
