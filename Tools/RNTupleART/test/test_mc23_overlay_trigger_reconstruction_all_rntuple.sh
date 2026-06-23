#!/bin/bash
#
# art-description: Reco_tf.py MC23 Overlay+Trigger+Reconstruction in RNTuple Format
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

HITS_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN3_2022[0])")
RDO_BKG_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_BKG_RUN3_2023[0])")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

# Convert the inputs first
HITSMerge_tf.py \
  --inputHITSFile="${HITS_File}" \
  --outputHITS_MRGFile="hits.rntuple.root" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? hits-conversion";

RDOMerge_tf.py \
  --PileUpPresampling="True" \
  --inputRDOFile="${RDO_BKG_File}" \
  --outputRDO_MRGFile="rdo_bkg.rntuple.root" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? rdobkg-conversion";

# Overlay+Trigger+Reconstruction
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="default:True" \
  --inputHITSFile="hits.rntuple.root" \
  --inputRDO_BKGFile="rdo_bkg.rntuple.root" \
  --outputAODFile="myAOD.pool.root" \
  --outputESDFile="myESD.pool.root" \
  --multithreaded="True" \
  --preInclude="all:Campaigns.MC23c" \
  --postInclude="default:PyJobTransforms.UseFrontier" \
  --skipEvents="0" \
  --autoConfiguration="everything" \
  --conditionsTag="default:${conditions}" \
  --geometryVersion="default:${geometry}" \
  --runNumber="601237" \
  --digiSeedOffset1="232" \
  --digiSeedOffset2="232" \
  --AMITag="r14799" \
  --steering "doOverlay" "doRDO_TRIG" "doTRIGtoALL" \
  --preExec="flags.Output.DefaultContainerType=\"ROOTRNTUPLE\";";
 
echo "art-result: $? full-chain";
