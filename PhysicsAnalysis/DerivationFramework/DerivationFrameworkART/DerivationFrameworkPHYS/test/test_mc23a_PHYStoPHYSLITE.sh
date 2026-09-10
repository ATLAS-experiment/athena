#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYStoPHYSLITE mc23a
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

set -e

Derivation_tf.py \
--inputDAOD_PHYSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7266/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p7266/DAOD_PHYS.49568713._001693.pool.root.1 \
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
