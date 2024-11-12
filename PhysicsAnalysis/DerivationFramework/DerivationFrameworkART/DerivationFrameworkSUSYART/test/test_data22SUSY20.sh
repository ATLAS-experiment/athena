#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building SUSY20 data22
# art-type: grid
# art-output: *.pool.root

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1 \
--outputDAODFile art.pool.root \
--formats SUSY20 \
--maxEvents -1 \

rc=$?
echo "art-result: $? derivation"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 3 ${ArtPackage} ${ArtJobName} --mode=semi-detailed
  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 regression"

exit $status