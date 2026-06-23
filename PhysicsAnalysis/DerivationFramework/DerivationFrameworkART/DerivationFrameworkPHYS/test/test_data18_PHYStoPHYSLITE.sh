#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYStoPHYSLITE data18
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputDAOD_PHYSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7266/data18_13TeV.00348885.physics_Main.deriv.DAOD_PHYS.r13286_p4910_p7266/DAOD_PHYS.49561712._000003.pool.root.1 \
--outputD2AODFile art.pool.root \
--formats PHYSLITE \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py D2AOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt

echo "art-result: $?  checkfile"

checkxAOD.py D2AOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py D2AOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1

echo "art-result: $?  checkIndexRefs"
