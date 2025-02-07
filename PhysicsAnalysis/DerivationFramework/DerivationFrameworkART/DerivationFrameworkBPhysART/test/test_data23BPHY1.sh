#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building BPHY1 data23
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1 \
--outputDAODFile art.pool.root \
--formats BPHY1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_BPHY1.art.pool.root > checkFile_BPHY1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY1.art.pool.root > checkxAOD_BPHY1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_BPHY1.art.pool.root > checkIndexRefs_BPHY1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
