#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1 data22
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

formats="EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1"

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1 \
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

