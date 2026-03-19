#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HIONHPOD data23PbPb
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/data23_hi.00463364.physics_HardProbes.AOD.r16069_p6447_skim \
--outputDAODFile art.pool.root \
--formats HIONHPOD \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HIONHPOD.art.pool.root > checkFile_HIONHPOD.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HIONHPOD.art.pool.root > checkxAOD_HIONHPOD.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HIONHPOD.art.pool.root > checkIndexRefs_HIONHPOD.txt 2>&1

echo "art-result: $?  checkIndexRefs"
