#!/bin/bash
#
# art-description: Derivation_tf.py mc23 w/ PHYS and PHYSLITE in TTree/RNTuple Formats w/ a diff of event data and metadata at the end
# art-type: grid
# art-include: main/Athena
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
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTTREEINDEX\";flags.Output.StorageTechnology.MetaData=\"ROOTTREE\";flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";

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
  --preExec="flags.Output.StorageTechnology.EventData={\"*\":\"ROOTRNTUPLE\"};flags.Output.StorageTechnology.MetaData=\"ROOTRNTUPLE\";";

echo "art-result: $? rntuple";

# RNTuple to TTree
timeout 64800 \
Merge_tf.py \
  --inputAODFile="DAOD_PHYS.rntuple.pool.root" \
  --outputAOD_MRGFile="DAOD_PHYS.rntuple-to-ttree.pool.root";

echo "art-result: $? conversion (PHYS)";

timeout 64800 \
Merge_tf.py \
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

# Metadata diff
meta-diff -d file_size file_guid auto_flush ".*eventTypes" --regex -m full -x diff -s DAOD_PHYS.ttree.pool.root DAOD_PHYS.rntuple.pool.root

echo "art-result: $? metadata diff (PHYS)";

meta-diff -d file_size file_guid auto_flush ".*eventTypes" --regex -m full -x diff -s DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.rntuple.pool.root

echo "art-result: $? metadata diff (PHYSLITE)";
