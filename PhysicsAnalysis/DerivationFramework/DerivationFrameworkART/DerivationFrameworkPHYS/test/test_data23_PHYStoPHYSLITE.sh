#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYStoPHYSLITE data23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")

set -e

Derivation_tf.py \
--inputDAOD_PHYSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7267/data23_13p6TeV.00456749.physics_Main.deriv.DAOD_PHYS.r15774_p6304_p7267/DAOD_PHYS.49630893._000015.pool.root.1 \
--outputD2AODFile art.pool.root \
--conditionsTag ${condition} \
--formats PHYSLITE \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py D2AOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt

echo "art-result: $?  checkfile"

checkxAOD.py D2AOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py D2AOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1

echo "art-result: $?  checkIndexRefs"
