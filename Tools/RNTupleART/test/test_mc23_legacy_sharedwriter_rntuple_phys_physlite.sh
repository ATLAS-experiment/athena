#!/bin/bash
#
# art-description: Derivation_tf.py mc23 w/ PHYS and PHYSLITE in RNTuple Format
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC[0])")

ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Derivation_tf.py \
  --multiprocess="True" \
  --sharedWriter="True" \
  --parallelCompression="False" \
  --inputAODFile="${AOD_File}" \
  --outputDAODFile="pool.root" \
  --formats "PHYS" "PHYSLITE" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? derivation";
