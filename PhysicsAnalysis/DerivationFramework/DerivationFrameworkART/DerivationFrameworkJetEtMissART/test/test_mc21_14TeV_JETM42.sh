#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building JETM42 mc21_14TeV_
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/AOD/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4422_r16129/AOD.41929775._000127.pool.root.1 \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats JETM42 \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_JETM42.art.pool.root > checkFile_JETM42.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM42.art.pool.root > checkxAOD_JETM42.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM42.art.pool.root > checkIndexRefs_JETM42.txt 2>&1

echo "art-result: $?  checkIndexRefs"
