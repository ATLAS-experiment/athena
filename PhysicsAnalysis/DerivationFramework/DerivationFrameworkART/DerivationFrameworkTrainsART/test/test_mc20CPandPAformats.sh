#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1 mc20
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

formats="EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1"

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1 \
--outputDAODFile art.pool.root \
--formats $formats \
--maxEvents -1 \

echo "art-result: $? reco"

function checkFormat()
{
  format=$1
  checkFile.py DAOD_${format}.art.pool.root > checkFile_${format}.txt
  echo "art-result: $?  checkfile $format"
  checkxAOD.py DAOD_${format}.art.pool.root > checkxAOD_${format}.txt
  echo "art-result: $?  checkxAOD $format"
  checkIndexRefs.py DAOD_${format}.art.pool.root > checkIndexRefs_${format}.txt 2>&1
  echo "art-result: $?  checkIndexRefs $format"
}

for f in $formats; do
    checkFormat $f;
done

