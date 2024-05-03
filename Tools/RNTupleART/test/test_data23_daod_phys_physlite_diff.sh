#!/bin/bash
#
# art-description: Derivation_tf.py data23 w/ PHYS and PHYSLITE in TTree/RNTuple Formats w/ a diff at the end
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="2000"

# TTree DAOD
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --CA="True" \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1" \
  --outputDAODFile="ttree.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTTREEINDEX\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

echo "art-result: $? ttree";

# RNTuple DAOD
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --CA="True" \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1" \
  --outputDAODFile="rntuple.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

echo "art-result: $? rntuple";

# RNTuple to TTree
timeout 64800 \
Merge_tf.py \
  --CA="True" \
  --inputAODFile="DAOD_PHYS.rntuple.pool.root" \
  --outputAOD_MRGFile="DAOD_PHYS.rntuple-to-ttree.pool.root";

echo "art-result: $? conversion (PHYS)";

timeout 64800 \
Merge_tf.py \
  --CA="True" \
  --inputAODFile="DAOD_PHYSLITE.rntuple.pool.root" \
  --outputAOD_MRGFile="DAOD_PHYSLITE.rntuple-to-ttree.pool.root";

echo "art-result: $? conversion (PHYSLITE)";

# Diff - See ATLASRECTS-7757 for non-default leaf list
acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.rntuple-to-ttree.pool.root;

echo "art-result: $? diff (PHYS)";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.rntuple-to-ttree.pool.root;

echo "art-result: $? diff (PHYSLITE)";
