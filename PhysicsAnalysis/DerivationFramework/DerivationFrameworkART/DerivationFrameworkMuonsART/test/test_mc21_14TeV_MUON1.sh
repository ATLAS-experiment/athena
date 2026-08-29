#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building MUON1 mc21_14TeV_
# art-type: grid
# art-memory: 4096
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/AOD/ATLAS-P2-RUN4-05-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4676_r17687/AOD.51454445._021552.pool.root.1 \
--outputDAODFile art.pool.root \
--formats MUON1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_MUON1.art.pool.root > checkFile_MUON1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_MUON1.art.pool.root > checkxAOD_MUON1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_MUON1.art.pool.root > checkIndexRefs_MUON1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
