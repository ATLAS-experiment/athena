#!/bin/sh
#
# art-description: Reco_tf runs on 2015 Heavy Ion data with MinBiasOverlay stream. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN2_DATA15_HI[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2_2015)")

Reco_tf.py --CA --multithreaded --maxEvents=25 --autoConfiguration 'everything' \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--preInclude="all:HIRecConfig.HIModeFlags.HImode" \
--preExec="flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False" \
--outputAODFile=myAOD.pool.root

RES=$?
echo "art-result: $RES Reco"
