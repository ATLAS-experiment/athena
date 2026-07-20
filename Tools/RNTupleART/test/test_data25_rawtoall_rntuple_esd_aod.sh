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

RAW_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA25[0])")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA25)")
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --inputBSFile="${RAW_File}" \
  --outputAODFile="myAOD.pool.root" \
  --outputESDFile="myESD.pool.root" \
  --multithreaded="True" \
  --autoConfiguration="everything" \
  --conditionsTag="all:${conditions}" \
  --geometryVersion="all:${geometry}" \
  --steering="doRAWtoALL" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? reconstruction";
