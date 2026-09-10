#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGSSV data23
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1 \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats FTAGSSV \
--maxEvents -1 

echo "art-result: $? reco"

checkFile.py DAOD_FTAGSSV.art.pool.root > checkFile_FTAGSSV.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAGSSV.art.pool.root > checkxAOD_FTAGSSV.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAGSSV.art.pool.root > checkIndexRefs_FTAGSSV.txt 2>&1

echo "art-result: $?  checkIndexRefs"
