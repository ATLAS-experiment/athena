#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS mc23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4162_r14622/1000events.AOD.33799166._000073.pool.root.1 \
--outputDAODFile art.pool.root \
--formats PHYS \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile_PHYS.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYS.art.pool.root > checkxAOD_PHYS.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYS.art.pool.root > checkIndexRefs_PHYS.txt 2>&1

echo "art-result: $?  checkIndexRefs"
