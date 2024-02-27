#!/bin/sh
#
# art-description: Test the reconstruction of Run 3 muon cosmic samples.
#
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

set -x

echo "List of files = " ${ArtInFile}

Reco_tf.py --CA True \
           --maxEvents=9000 \
           --conditionsTag CONDBR2-BLKPA-2022-12  \
           --geometryVersion ATLAS-R3S-2021-03-02-01 \
           --inputBSFile='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/MuonCosmic/data22_cos.00415278.physics_CosmicMuons.merge.RAW._lb0002._SFO-ALL._0001.1' \
           --outputAODFile MuonCosmic_Reco.AOD.pool.root \
           --preExec 'flags.Trigger.doLVL1=False;flags.Trigger.doHLT=False'

exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"