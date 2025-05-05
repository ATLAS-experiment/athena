#!/bin/sh
#
# art-description: Reco_tf runs on 2024 Heavy Ion data with Minbias stream (UPC mode). Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA24_HI[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA24)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

Reco_tf.py --CA --multithreaded --maxEvents=50 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--preExec="flags.Reco.HIMode=HIMode.UPC" \
--outputAODFile=myAOD.pool.root

RES=$?
echo "art-result: $RES Reco"
