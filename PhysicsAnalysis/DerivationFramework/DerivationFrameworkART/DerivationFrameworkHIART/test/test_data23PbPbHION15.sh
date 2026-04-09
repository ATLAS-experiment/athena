#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION15 data23PbPb
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/data23_hi.00463364.physics_HardProbes.AOD.r16069_p6447_skim \
--outputDAODFile art.pool.root \
--formats HION15 \
--maxEvents -1 \
--preExec 'flags.HeavyIon.isDerivation=True' \

echo "art-result: $? reco"

checkFile.py DAOD_HION15.art.pool.root > checkFile_HION15.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION15.art.pool.root > checkxAOD_HION15.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION15.art.pool.root > checkIndexRefs_HION15.txt 2>&1

echo "art-result: $?  checkIndexRefs"
