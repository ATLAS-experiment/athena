#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYS w/ LLP1 and FTAG2 augmentations - AthenaMP + SW w/o parallel compression
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-athena-mt: 8

ATHENA_CORE_NUMBER=8 Derivation_tf.py \
  --preExec 'flags.Output.TreeAutoFlush={"DAOD_PHYS": 100}' \
  --maxEvents '1000' \
  --multiprocess 'True' \
  --sharedWriter 'True' \
  --parallelCompression 'False' \
  --inputAODFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1' \
  --outputDAODFile 'art.pool.root' \
  --formats 'PHYS' 'LLP1' 'FTAG2' \
  --augmentations 'LLP1:PHYS' 'FTAG2:PHYS'

echo "art-result: $? reco"

checkFile.py DAOD_PHYS.art.pool.root > checkFile.txt

echo "art-result: $? checkfile"
