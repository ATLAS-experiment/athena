#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HIONHPOD mc23PbPb
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/mc23_5p36TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.AOD.e8599_s4576_s4483_r16930_skim.root \
--outputDAODFile art.pool.root \
--formats HIONHPOD \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_HIONHPOD.art.pool.root > checkFile_HIONHPOD.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HIONHPOD.art.pool.root > checkxAOD_HIONHPOD.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HIONHPOD.art.pool.root > checkIndexRefs_HIONHPOD.txt 2>&1

echo "art-result: $?  checkIndexRefs"
