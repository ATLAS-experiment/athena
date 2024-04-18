#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS and PHYSLITE data18 MP w/ SharedWriter
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt
# art-output: readDataHeader*.txt
# art-athena-mt: 4

ATHENA_CORE_NUMBER=4 Derivation_tf.py \
  --CA True \
  --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1 \
  --outputDAODFile art.pool.root \
  --formats PHYS PHYSLITE \
  --maxEvents -1 \
  --sharedWriter True \
  --multiprocess True \

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile_PHYS.txt

echo "art-result: $?  checkfile PHYS"

checkFile.py DAOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt

echo "art-result: $?  checkfile PHYSLITE"

checkxAOD.py DAOD_PHYS.art.pool.root > checkxAOD_PHYS.txt

echo "art-result: $?  checkxAOD PHYS"

checkxAOD.py DAOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt

echo "art-result: $?  checkxAOD PHYSLITE"

checkIndexRefs.py DAOD_PHYS.art.pool.root > checkIndexRefs_PHYS.txt 2>&1

echo "art-result: $?  checkIndexRefs PHYS"

checkIndexRefs.py DAOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1

echo "art-result: $?  checkIndexRefs PHYSLITE"

readDataHeader.py --filesInput=DAOD_PHYS.art.pool.root > readDataHeader_PHYS.txt 2>&1
tail readDataHeader_PHYS.txt | grep -q "Application Manager Terminated successfully"

echo "art-result: $? readDataHeader PHYS"

readDataHeader.py --filesInput=DAOD_PHYSLITE.art.pool.root > readDataHeader_PHYSLITE.txt 2>&1
tail readDataHeader_PHYSLITE.txt | grep -q "Application Manager Terminated successfully"

echo "art-result: $? readDataHeader PHYSLITE"

checkMetadata.py --filesInput=DAOD_PHYS.art.pool.root > checkMetadata_PHYS.txt 2>&1

echo "art-result: $? checkMetadata PHYS"

checkMetadata.py --filesInput=DAOD_PHYSLITE.art.pool.root > checkMetadata_PHYSLITE.txt 2>&1

echo "art-result: $? checkMetadata PHYSLITE"
