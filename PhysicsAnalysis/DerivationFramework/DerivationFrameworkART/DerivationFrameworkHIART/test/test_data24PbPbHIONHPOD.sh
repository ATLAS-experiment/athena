#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HIONHPOD data24PbPb
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/data24_hi.00490145.physics_HardProbes.AOD.f1550_m2267_skim \
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
