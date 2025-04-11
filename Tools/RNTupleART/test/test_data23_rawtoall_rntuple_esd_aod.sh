#!/bin/bash
#
# art-description: Reco_tf.py data23 RAWtoALL w/ AOD+ESD in RNTuple Format
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

NEVENTS="540"

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --maxEvents="${NEVENTS}" \
  --inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data" \
  --outputAODFile="myAOD.pool.root" \
  --outputESDFile="myESD.pool.root" \
  --multithreaded="True" \
  --autoConfiguration="everything" \
  --conditionsTag="all:${conditionsTag}" \
  --geometryVersion="all:ATLAS-R3S-2021-03-02-00" \
  --steering="doRAWtoALL" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? reconstruction";
