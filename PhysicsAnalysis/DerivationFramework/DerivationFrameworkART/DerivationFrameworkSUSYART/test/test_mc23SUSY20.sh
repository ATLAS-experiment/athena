#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building SUSY20 mc23
# art-type: grid
# art-output: *.pool.root

set -e

Derivation_tf.py \
--inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4162_r14622/1000events.AOD.33799166._000073.pool.root.1 \
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