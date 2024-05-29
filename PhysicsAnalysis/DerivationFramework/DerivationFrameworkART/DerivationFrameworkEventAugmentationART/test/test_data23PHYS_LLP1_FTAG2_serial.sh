#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS w/ LLP1 and FTAG2 augmentations - serial Athena
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt

Derivation_tf.py \
  --maxEvents '2000' \
  --inputAODFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1' \
  --outputDAODFile 'art.pool.root' \
  --formats 'PHYS' 'LLP1' 'FTAG2' \
  --augmentations 'LLP1:PHYS' 'FTAG2:PHYS'

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile.txt

echo "art-result: $? checkfile"
