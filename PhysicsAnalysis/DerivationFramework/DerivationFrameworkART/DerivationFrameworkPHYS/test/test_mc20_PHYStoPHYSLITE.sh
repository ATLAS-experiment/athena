#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYStoPHYSLITE mc20
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")

set -e

Derivation_tf.py \
--inputDAOD_PHYSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7266/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13145_r13146_p7266/DAOD_PHYS.49680245._000770.pool.root.1 \
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
