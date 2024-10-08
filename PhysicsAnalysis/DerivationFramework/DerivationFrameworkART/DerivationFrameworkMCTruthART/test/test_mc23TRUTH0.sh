#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building TRUTH0 mc23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1 \
--outputDAODFile art.pool.root \
--formats TRUTH0 \
--maxEvents 1000

echo "art-result: $? reco"

checkFile.py DAOD_TRUTH0.art.pool.root > checkFile_TRUTH0.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRUTH0.art.pool.root > checkxAOD_TRUTH0.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRUTH0.art.pool.root > checkIndexRefs_TRUTH0.txt 2>&1

echo "art-result: $?  checkIndexRefs"
