#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYSLITE mc21_14TeV_
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
--formats PHYSLITE \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1

echo "art-result: $?  checkIndexRefs"
