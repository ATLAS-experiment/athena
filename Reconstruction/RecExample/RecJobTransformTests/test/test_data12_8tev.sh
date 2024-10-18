#!/bin/sh
#
# art-description: Reco_tf runs on 2012 8 TeV collision data with all streams. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN1_DATA12[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN1_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN1_2012)")

Reco_tf.py  --CA --multithreaded --maxEvents 300 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root 

RES=$?
echo "art-result: $RES Reco"
