#!/bin/bash
#
# art-description: Derivation_tf.py mc23 w/ PHYS and PHYSLITE in TTree/RNTuple Formats w/ a metadata diff at the end
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="1000"

# TTree DAOD
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1" \
  --outputDAODFile="ttree.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.StorageTechnology.MetaData=\"ROOTTREE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

echo "art-result: $? ttree";

# RNTuple DAOD
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1" \
  --outputDAODFile="rntuple.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.StorageTechnology.MetaData=\"ROOTRNTUPLE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

echo "art-result: $? rntuple";

# Only subset of metadata keys are compared here since support for metatadata in RNTuple is currently limited
compare_metadata.py \
--ttree-file-path DAOD_PHYS.ttree.pool.root \
--rntuple-file-path DAOD_PHYS.rntuple.pool.root \
--keys-to-compare nentries itemList eventTypes runNumbers lumiBlockNumbers processingTags FileMetaData \
--fmd-keys-to-compare amiTag beamEnergy beamType conditionsTag dataYear geometryVersion mcProcID

echo "art-result: $? diff (PHYS)";

compare_metadata.py \
--ttree-file-path DAOD_PHYSLITE.ttree.pool.root \
--rntuple-file-path DAOD_PHYSLITE.rntuple.pool.root \
--keys-to-compare nentries itemList eventTypes runNumbers lumiBlockNumbers processingTags FileMetaData \
--fmd-keys-to-compare amiTag beamEnergy beamType conditionsTag dataYear geometryVersion mcProcID

echo "art-result: $? diff (PHYSLITE)";
