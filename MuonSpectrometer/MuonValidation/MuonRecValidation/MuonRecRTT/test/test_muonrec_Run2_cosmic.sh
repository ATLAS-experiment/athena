#!/bin/sh
#
# art-description: Test the reconstruction of muon cosmic samples.
#
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena

set -x

echo "List of files = " ${ArtInFile}

Reco_tf.py --CA True \
           --maxEvents=9000 \
           --conditionsTag RAWtoESD:CONDBR2-BLKPA-RUN2-09  \
           --geometryVersion ATLAS-R2-2016-01-00-01 \
           --inputBSFile='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/MuonCosmic/data17_cos.00342172.physics_CosmicMuons.merge.RAW._lb0006._SFO-ALL._0001.1' \
           --outputAODFile=MuonCosmic_Reco.AOD.pool.root \
           --preExec 'flags.Trigger.doLVL1=False;flags.Trigger.doHLT=False'

exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"