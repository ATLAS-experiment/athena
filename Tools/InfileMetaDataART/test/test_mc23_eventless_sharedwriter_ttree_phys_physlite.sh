#!/bin/bash
#
# art-description: Derivation_tf.py mc23 w/ PHYS and PHYSLITE in TTree Format using sharedWriter, diffs of event/meta-data when eventless worker
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

##################################################################################
# KNOWN ISSUES TRACKED BY THIS TEST:
#
# 1. Values for FileMetaData fields of output are same as input file for shared
#    writer with eventless worker
#    - mc23: productionRelease, dataType, lumiBlocks
##################################################################################

NEVENTS="2"
INPUTAODFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1"

# TTree DAOD MP False SW False, baseline for comparison
ATHENA_CORE_NUMBER=2 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="False" \
  --sharedWriter="False" \
  --parallelCompression="False" \
  --inputAODFile="${INPUTAODFILE}" \
  --outputDAODFile="ttree.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";\

echo "art-result: $? ttree MP False SW False";

# TTree DAOD MP True SW True, all workers process at least one event based on RoundRobin strategy
ATHENA_CORE_NUMBER=2 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --athenaMPStrategy="RoundRobin" \
  --parallelCompression="False" \
  --inputAODFile="${INPUTAODFILE}" \
  --outputDAODFile="ttree_AllWorkersHaveEvent.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";\

echo "art-result: $? ttree MP True SW True AllWorkersHaveEvent";

# TTree DAOD MP True SW True, one worker will be eventless since ChunkSize==maxEvents

### Notes: Setting the ChunkSize to be explicit, technically not needed as it would otherwise be set based on auto_flush of input file (which is 100 in this case)
### Using EventOrders to set a worker to process 0 events doesn't seem to work
### The resulting output seems to be the same regardless of the order (whether the first or second worker was eventless)

ATHENA_CORE_NUMBER=2 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="${INPUTAODFILE}" \
  --outputDAODFile="ttree_EventlessWorker.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.MP.ChunkSize=${NEVENTS};flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";\

echo "art-result: $? ttree MP True SW True EventlessWorker";

# Diff
acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? diff PHYS sharedWriter AllWorkersHaveEvent";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker.pool.root;

echo "art-result: $? diff PHYS sharedWriter EventlessWorker";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? diff PHYSLITE sharedWriter AllWorkersHaveEvent";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker.pool.root;

echo "art-result: $? diff PHYSLITE sharedWriter EventlessWorker";

# Metadata diff

METADATA_FIELDS_TO_IGNORE="file_size file_guid auto_flush .*eventTypes"
METADATA_FIELDS_TO_IGNORE_EVENTLESS="file_size file_guid auto_flush .*eventTypes"

### Ignore FileMetaData fields that are not set correctly when eventless worker is involved (to be removed when fixed)
FILEMETADATA_FIELDS_TO_IGNORE="FileMetaData.dataType FileMetaData.lumiBlocks FileMetaData.productionRelease"
echo "WARNING These FileMetaData fields will be ignored in diff for EventlessWorker: $FILEMETADATA_FIELDS_TO_IGNORE"
METADATA_FIELDS_TO_IGNORE_EVENTLESS="$METADATA_FIELDS_TO_IGNORE_EVENTLESS $FILEMETADATA_FIELDS_TO_IGNORE"

meta-diff -d $METADATA_FIELDS_TO_IGNORE --regex -m full -x diff -s \
  DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? metadata diff PHYS sharedWriter AllWorkersHaveEvent";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_EVENTLESS --regex -m full -x diff -s \
  DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker.pool.root;

echo "art-result: $? metadata diff PHYS sharedWriter EventlessWorker";

meta-diff -d $METADATA_FIELDS_TO_IGNORE --regex -m full -x diff -s \
  DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? metadata diff PHYSLITE sharedWriter AllWorkersHaveEvent";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_EVENTLESS --regex -m full -x diff -s \
  DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker.pool.root;

echo "art-result: $? metadata diff PHYSLITE sharedWriter EventlessWorker";
