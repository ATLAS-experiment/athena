#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TCAL1 mc23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1 \
--outputDAODFile art.pool.root \
--formats TCAL1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_TCAL1.art.pool.root > checkFile_TCAL1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TCAL1.art.pool.root > checkxAOD_TCAL1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TCAL1.art.pool.root > checkIndexRefs_TCAL1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
