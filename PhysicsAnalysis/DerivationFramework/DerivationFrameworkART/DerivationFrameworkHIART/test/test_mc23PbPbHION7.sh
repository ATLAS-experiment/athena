#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building HION7 mc23PbPb
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/mc23_5p36TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.AOD.e8599_s4576_s4483_r16930_skim.root \
--outputDAODFile art.pool.root \
--formats HION7 \
--maxEvents -1 \
--multithreaded true \
--preExec 'flags.HeavyIon.isDerivation=True' \

echo "art-result: $? reco"

checkFile.py DAOD_HION7.art.pool.root > checkFile_HION7.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION7.art.pool.root > checkxAOD_HION7.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HION7.art.pool.root > checkIndexRefs_HION7.txt 2>&1

echo "art-result: $?  checkIndexRefs"
