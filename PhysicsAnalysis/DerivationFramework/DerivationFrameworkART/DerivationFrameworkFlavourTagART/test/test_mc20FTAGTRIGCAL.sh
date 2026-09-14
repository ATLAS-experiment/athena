#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGTRIGCAL mc20
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN2_MC[0])")

set -e

Derivation_tf.py \
--inputAODFile ${AOD_File} \
--outputDAODFile art.pool.root \
--formats FTAGTRIGCAL \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_FTAGTRIGCAL.art.pool.root > checkFile_FTAGTRIGCAL.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAGTRIGCAL.art.pool.root > checkxAOD_FTAGTRIGCAL.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAGTRIGCAL.art.pool.root > checkIndexRefs_FTAGTRIGCAL.txt 2>&1

echo "art-result: $?  checkIndexRefs"
