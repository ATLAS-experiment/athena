#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building SUSY20 data18
# art-type: grid
# art-output: *.pool.root

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1 \
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