#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 JETM6 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1 data22
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1 \
--outputDAODFile art.pool.root \
--formats EGAM1 EGAM2 EGAM3 EGAM4 EGAM5 EGAM7 EGAM8 EGAM9 EGAM10 JETM1 JETM3 JETM4 JETM6 FTAG1 FTAG2 FTAG3 IDTR2 TRIG8 LLP1 STDM7 STDM13 HIGG1D1 MUON1 \
--maxEvents -1 \

echo "art-result: $? reco"

checkFile.py DAOD_EGAM1.art.pool.root > checkFile_EGAM1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM1.art.pool.root > checkxAOD_EGAM1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM1.art.pool.root > checkIndexRefs_EGAM1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM2.art.pool.root > checkFile_EGAM2.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM2.art.pool.root > checkxAOD_EGAM2.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM2.art.pool.root > checkIndexRefs_EGAM2.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM3.art.pool.root > checkFile_EGAM3.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM3.art.pool.root > checkxAOD_EGAM3.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM3.art.pool.root > checkIndexRefs_EGAM3.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM4.art.pool.root > checkFile_EGAM4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM4.art.pool.root > checkxAOD_EGAM4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM4.art.pool.root > checkIndexRefs_EGAM4.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM5.art.pool.root > checkFile_EGAM5.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM5.art.pool.root > checkxAOD_EGAM5.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM5.art.pool.root > checkIndexRefs_EGAM5.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM7.art.pool.root > checkFile_EGAM7.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM7.art.pool.root > checkxAOD_EGAM7.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM7.art.pool.root > checkIndexRefs_EGAM7.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM8.art.pool.root > checkFile_EGAM8.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM8.art.pool.root > checkxAOD_EGAM8.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM8.art.pool.root > checkIndexRefs_EGAM8.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM9.art.pool.root > checkFile_EGAM9.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM9.art.pool.root > checkxAOD_EGAM9.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM9.art.pool.root > checkIndexRefs_EGAM9.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_EGAM10.art.pool.root > checkFile_EGAM10.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM10.art.pool.root > checkxAOD_EGAM10.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_EGAM10.art.pool.root > checkIndexRefs_EGAM10.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM1.art.pool.root > checkFile_JETM1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM1.art.pool.root > checkxAOD_JETM1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM1.art.pool.root > checkIndexRefs_JETM1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM3.art.pool.root > checkFile_JETM3.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM3.art.pool.root > checkxAOD_JETM3.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM3.art.pool.root > checkIndexRefs_JETM3.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM4.art.pool.root > checkFile_JETM4.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM4.art.pool.root > checkxAOD_JETM4.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM4.art.pool.root > checkIndexRefs_JETM4.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_JETM6.art.pool.root > checkFile_JETM6.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_JETM6.art.pool.root > checkxAOD_JETM6.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_JETM6.art.pool.root > checkIndexRefs_JETM6.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_FTAG1.art.pool.root > checkFile_FTAG1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAG1.art.pool.root > checkxAOD_FTAG1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAG1.art.pool.root > checkIndexRefs_FTAG1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_FTAG2.art.pool.root > checkFile_FTAG2.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAG2.art.pool.root > checkxAOD_FTAG2.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAG2.art.pool.root > checkIndexRefs_FTAG2.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_FTAG3.art.pool.root > checkFile_FTAG3.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_FTAG3.art.pool.root > checkxAOD_FTAG3.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_FTAG3.art.pool.root > checkIndexRefs_FTAG3.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_IDTR2.art.pool.root > checkFile_IDTR2.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_IDTR2.art.pool.root > checkxAOD_IDTR2.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_IDTR2.art.pool.root > checkIndexRefs_IDTR2.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_TRIG8.art.pool.root > checkFile_TRIG8.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRIG8.art.pool.root > checkxAOD_TRIG8.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRIG8.art.pool.root > checkIndexRefs_TRIG8.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_TRIG9.art.pool.root > checkFile_TRIG9.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_TRIG9.art.pool.root > checkxAOD_TRIG9.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_TRIG9.art.pool.root > checkIndexRefs_TRIG9.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_LLP1.art.pool.root > checkFile_LLP1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_LLP1.art.pool.root > checkxAOD_LLP1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_LLP1.art.pool.root > checkIndexRefs_LLP1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_STDM7.art.pool.root > checkFile_STDM7.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_STDM7.art.pool.root > checkxAOD_STDM7.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_STDM7.art.pool.root > checkIndexRefs_STDM7.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_STDM13.art.pool.root > checkFile_STDM13.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_STDM13.art.pool.root > checkxAOD_STDM13.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_STDM13.art.pool.root > checkIndexRefs_STDM13.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_HIGG1D1.art.pool.root > checkFile_HIGG1D1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HIGG1D1.art.pool.root > checkxAOD_HIGG1D1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_HIGG1D1.art.pool.root > checkIndexRefs_HIGG1D1.txt 2>&1

echo "art-result: $?  checkIndexRefs"

checkFile.py DAOD_MUON1.art.pool.root > checkFile_MUON1.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_MUON1.art.pool.root > checkxAOD_MUON1.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_MUON1.art.pool.root > checkIndexRefs_MUON1.txt 2>&1

echo "art-result: $?  checkIndexRefs"
