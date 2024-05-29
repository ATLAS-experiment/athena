#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS w/ LLP1 and FTAG2 augmentations - AthenaMP + SW w/o parallel compression
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-athena-mt: 8

ATHENA_CORE_NUMBER=8 Derivation_tf.py \
  --preExec 'flags.Output.TreeAutoFlush={"DAOD_PHYS": 100}' \
  --maxEvents '2000' \
  --multiprocess 'True' \
  --sharedWriter 'True' \
  --parallelCompression 'False' \
  --inputAODFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1' \
  --outputDAODFile 'art.pool.root' \
  --formats 'PHYS' 'LLP1' 'FTAG2' \
  --augmentations 'LLP1:PHYS' 'FTAG2:PHYS'

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile.txt

echo "art-result: $? checkfile"
