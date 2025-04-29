#!/bin/sh

# Disable this test: Derivations are not expected to work with MT.
# At a minimum, the truth dressing tool and overlap removal need to
# be redesigned to avoid modifying the same decoration from different
# algorithms.
# art-include: main/Athena_disabled

# art-include: 21.2/AthDerivation
# art-description: DAOD building PHYSVAL data18 MT (DISABLED)
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt
# art-output: checkIndexRefs*.txt
# art-athena-mt: 1

set -e

ATHENA_CORE_NUMBER=1
Derivation_tf.py \
    --multithreaded \
    --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1 \
    --outputDAODFile art.pool.root \
    --formats PHYSVAL \
    --maxEvents -1

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_PHYSVALFile DAOD_PHYSVAL.art.pool.root --outputDAOD_PHYSVAL_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_PHYSVAL.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYSVAL.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYSVAL.art.pool.root > checkIndexRefs_PHYSVAL.txt 2>&1

echo "art-result: $?  checkIndexRefs PHYSVAL"
