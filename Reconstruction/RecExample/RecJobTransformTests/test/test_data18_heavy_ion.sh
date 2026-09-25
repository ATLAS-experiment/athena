#!/bin/sh
#
# art-description: Reco_tf runs on 2018 Heavy Ion data with HardProbes stream. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
export TRF_ECHO=True;
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN2_DATA18_HI[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")

Reco_tf.py --multithreaded --maxEvents=20 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--steering='doRAWtoALL' \
--preInclude="all:HIRecConfig.HIModeFlags.HImode" \
--preExec="flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False" \
--outputAODFile=AOD.pool.root

RES=$?
echo "art-result: $RES Reco"
