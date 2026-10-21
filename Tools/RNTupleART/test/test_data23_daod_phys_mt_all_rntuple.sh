#!/bin/bash
#
# art-description: Derivation_tf.py data23 w/ PHYS in AthenaMT
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_DATA[0])")
AOD_MRG_File="aod.rntuple.root"
DAOD_File="daod.rntuple.root"

# Convert the inputs first
AODMerge_tf.py \
  --inputAODFile="${AOD_File}" \
  --outputAOD_MRGFile="${AOD_MRG_File}" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? aod-conversion";

# RNTuple AOD -> DAOD
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --multithreaded="True" \
  --inputAODFile="${AOD_MRG_File}" \
  --outputDAODFile="${DAOD_File}" \
  --formats "PHYS" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";\

echo "art-result: $? rntuple";
