#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAGSSV data24
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA24)")

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data24/AOD/data24_13p6TeV.00486658.physics_Main.recon.AOD.f1522_m2262_r16385_r16377/AOD.43718985._000221.pool.root.1 \
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
