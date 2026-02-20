#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building FTAG5 mc21_14TeV_
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/AOD/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4494_r16436/AOD.44098360._000011.pool.root.1 \
--outputDAODFile art.pool.root \
--formats FTAG5 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_FTAG5.art.pool.root > checkFile_FTAG5.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAG5.art.pool.root > checkxAOD_FTAG5.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAG5.art.pool.root > checkIndexRefs_FTAG5.txt 2>&1

echo "art-result: $?  checkIndexRefs"
