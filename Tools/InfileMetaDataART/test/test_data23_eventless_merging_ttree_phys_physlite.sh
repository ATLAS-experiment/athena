#!/bin/bash
#
# art-description: Derivation_tf.py data23 w/ PHYS and PHYSLITE in TTree Format with MP and merging of worker outputs, diffs of event/meta-data when eventless worker
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
# 1. FileMetaData fields have empty values if the first merging input is eventless
#    - data23: amiTag, beamEnergy, beamType, geometryVersion, productionRelease
# 2. dataYear field is absent after merging with eventless file
##################################################################################

NEVENTS="2"
INPUTAODFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1"

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

# TTree DAOD MP True SW False, all workers process at least one event based on RoundRobin strategy
ATHENA_CORE_NUMBER=2 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="False" \
  --athenaMPStrategy="RoundRobin" \
  --parallelCompression="False" \
  --inputAODFile="${INPUTAODFILE}" \
  --outputDAODFile="ttree_AllWorkersHaveEvent.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";\

echo "art-result: $? ttree MP True SW False AllWorkersHaveEvent";

# TTree DAOD MP True SW False, one worker will be eventless since ChunkSize==maxEvents

### Notes: Setting the ChunkSize to be explicit, technically not needed as it would otherwise be set based on auto_flush of input file (which is 100 in this case)
### Using EventOrders to set a worker to process 0 events doesn't seem to work
### The FileMetaData depends on whether the first file is eventless for merging; disable merging here so that we merge in a separate step to control the file order

ATHENA_CORE_NUMBER=2 \
timeout 64800 \
Derivation_tf.py \
  --maxEvents="${NEVENTS}" \
  --multiprocess="True" \
  --sharedWriter="False" \
  --athenaMPMergeTargetSize="DAOD*:0" \
  --parallelCompression="False" \
  --inputAODFile="${INPUTAODFILE}" \
  --outputDAODFile="ttree_EventlessWorker.pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.MP.ChunkSize=${NEVENTS};flags.Output.TreeAutoFlush={\"DAOD_PHYS\": 100, \"DAOD_PHYSLITE\": 100};";\

echo "art-result: $? ttree MP True SW False EventlessWorker";

### Assume one worker processed all events designated by : in event orders file
N_WORKER_WITH_EVENTS=$(awk -F: '/:/{print $1; exit}' athenamp_eventorders.txt.Derivation)
N_WORKER_MAX=$(awk -F: '{print $1}' athenamp_eventorders.txt.Derivation | sort -n | tail -1)

### Make a list of worker output files excluding the one with events
PHYS_WITHOUT_EVENTS=$(for i in $(seq 1 $((N_WORKER_MAX + 1))); do ((i != N_WORKER_WITH_EVENTS + 1)) && printf "DAOD_PHYS.ttree_EventlessWorker.pool.root_%03d " "$i"; done)
PHYS_WITH_EVENTS="DAOD_PHYS.ttree_EventlessWorker.pool.root_$(printf "%03d" $((N_WORKER_WITH_EVENTS + 1)))"
PHYSLITE_WITHOUT_EVENTS=$(for i in $(seq 1 $((N_WORKER_MAX + 1))); do ((i != N_WORKER_WITH_EVENTS + 1)) && printf "DAOD_PHYSLITE.ttree_EventlessWorker.pool.root_%03d " "$i"; done)
PHYSLITE_WITH_EVENTS="DAOD_PHYSLITE.ttree_EventlessWorker.pool.root_$(printf "%03d" $((N_WORKER_WITH_EVENTS + 1)))"

# Merging when the first input has events
timeout 64800 \
Merge_tf.py \
  --inputAODFile $PHYS_WITH_EVENTS $PHYS_WITHOUT_EVENTS \
  --outputAOD_MRGFile="DAOD_PHYS.ttree_EventlessWorker_FirstHasEvents.pool.root";

echo "art-result: $? merging first input has events (PHYS)";

timeout 64800 \
Merge_tf.py \
  --inputAODFile $PHYSLITE_WITH_EVENTS $PHYSLITE_WITHOUT_EVENTS \
  --outputAOD_MRGFile="DAOD_PHYSLITE.ttree_EventlessWorker_FirstHasEvents.pool.root";

echo "art-result: $? merging first input has events (PHYSLITE)";


# Merging when the first input has no events
timeout 64800 \
Merge_tf.py \
  --inputAODFile $PHYS_WITHOUT_EVENTS $PHYS_WITH_EVENTS \
  --outputAOD_MRGFile="DAOD_PHYS.ttree_EventlessWorker_FirstNoEvents.pool.root";

echo "art-result: $? merging first input no events (PHYS)";

timeout 64800 \
Merge_tf.py \
  --inputAODFile $PHYSLITE_WITHOUT_EVENTS $PHYSLITE_WITH_EVENTS \
  --outputAOD_MRGFile="DAOD_PHYSLITE.ttree_EventlessWorker_FirstNoEvents.pool.root";

echo "art-result: $? merging first input no events (PHYSLITE)";


# Diff
acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? diff PHYS AllWorkersHaveEvent";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker_FirstHasEvents.pool.root;

echo "art-result: $? diff PHYS EventlessWorker FirstHasEvents";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker_FirstNoEvents.pool.root;

echo "art-result: $? diff PHYS EventlessWorker FirstNoEvents";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? diff PHYSLITE AllWorkersHaveEvent";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker_FirstHasEvents.pool.root;

echo "art-result: $? diff PHYSLITE EventlessWorker FirstHasEvents";

acmd diff-root \
  --ignore-leaves 'index_ref' '(.*)_timings\.(.*)' '(.*)_mems\.(.*)' '(.*)TrigCostContainer(.*)' '(.*)DFCommonJets(.*)fJvt' \
  --nan-equal \
  --exact-branches \
  --order-trees DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker_FirstNoEvents.pool.root;

echo "art-result: $? diff PHYSLITE EventlessWorker FirstNoEvents";

# Metadata diff

METADATA_FIELDS_TO_IGNORE="file_size file_guid auto_flush .*eventTypes"
METADATA_FIELDS_TO_IGNORE_FIRST_HAS_EVENTS="file_size file_guid auto_flush .*eventTypes"
METADATA_FIELDS_TO_IGNORE_FIRST_NO_EVENTS="file_size file_guid auto_flush .*eventTypes"

### Ignore FileMetaData fields that are not set correctly when eventless worker is involved (to be removed when fixed)
### For the case of when merging where the first input has no events, FileMetaData fields will have empty values
### dataYear is missing completely so we ignore entire FileMetaData and metadata_items 
FILEMETADATA_FIELDS_TO_IGNORE="FileMetaData metadata_items"
echo "WARNING These FileMetaData fields will be ignored in diff for EventlessWorker FirstNoEvents: $FILEMETADATA_FIELDS_TO_IGNORE"
METADATA_FIELDS_TO_IGNORE_FIRST_NO_EVENTS="$METADATA_FIELDS_TO_IGNORE_FIRST_NO_EVENTS $FILEMETADATA_FIELDS_TO_IGNORE"

meta-diff -d $METADATA_FIELDS_TO_IGNORE --regex -m full -x diff -s \
  DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? metadata diff PHYS AllWorkersHaveEvent";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_FIRST_HAS_EVENTS --regex -m full -x diff -s \
  DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker_FirstHasEvents.pool.root;

echo "art-result: $? metadata diff PHYS EventlessWorker FirstHasEvents";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_FIRST_NO_EVENTS --regex -m full -x diff -s \
  DAOD_PHYS.ttree.pool.root DAOD_PHYS.ttree_EventlessWorker_FirstNoEvents.pool.root;

echo "art-result: $? metadata diff PHYS EventlessWorker FirstNoEvents";

meta-diff -d $METADATA_FIELDS_TO_IGNORE --regex -m full -x diff -s \
  DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_AllWorkersHaveEvent.pool.root;

echo "art-result: $? metadata diff PHYSLITE AllWorkersHaveEvent";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_FIRST_HAS_EVENTS --regex -m full -x diff -s \
  DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker_FirstHasEvents.pool.root;

echo "art-result: $? metadata diff PHYSLITE EventlessWorker FirstHasEvents";

meta-diff -d $METADATA_FIELDS_TO_IGNORE_FIRST_NO_EVENTS --regex -m full -x diff -s \
  DAOD_PHYSLITE.ttree.pool.root DAOD_PHYSLITE.ttree_EventlessWorker_FirstNoEvents.pool.root;

echo "art-result: $? metadata diff PHYSLITE EventlessWorker FirstNoEvents";
