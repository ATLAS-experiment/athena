#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION5 data24PbPb
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/data24_hi.00490145.physics_HardProbes.AOD.f1550_m2267_skim \
--outputDAODFile art.pool.root \
--formats HION5 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HION5.art.pool.root > checkFile_HION5.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION5.art.pool.root > checkxAOD_HION5.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION5.art.pool.root > checkIndexRefs_HION5.txt 2>&1

echo "art-result: $?  checkIndexRefs"
