#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TRUTH3 mc23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

EVNT_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.EVNT_RUN3_1K[0])")

Derivation_tf.py \
--inputEVNTFile ${EVNT_File} \
--outputDAODFile art.pool.root \
--formats TRUTH3 \
--maxEvents 1000

echo "art-result: $? reco"

checkFile.py DAOD_TRUTH3.art.pool.root > checkFile_TRUTH3.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRUTH3.art.pool.root > checkxAOD_TRUTH3.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRUTH3.art.pool.root > checkIndexRefs_TRUTH3.txt 2>&1

echo "art-result: $?  checkIndexRefs"
