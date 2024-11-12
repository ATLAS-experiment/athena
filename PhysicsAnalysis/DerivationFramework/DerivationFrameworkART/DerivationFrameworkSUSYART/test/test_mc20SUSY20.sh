#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building SUSY20 mc20
# art-type: grid
# art-output: *.pool.root

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1 \
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