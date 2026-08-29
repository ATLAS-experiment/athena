#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building BPHY4 mc21_14TeV_
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
--formats BPHY4 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_BPHY4.art.pool.root > checkFile_BPHY4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_BPHY4.art.pool.root > checkxAOD_BPHY4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_BPHY4.art.pool.root > checkIndexRefs_BPHY4.txt 2>&1

echo "art-result: $?  checkIndexRefs"
