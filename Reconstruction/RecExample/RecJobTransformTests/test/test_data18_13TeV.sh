#!/bin/sh
#
# art-description: Reco_tf runs on 2018 13 TeV collision data with all streams. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN2_DATA18[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")

Reco_tf.py --CA --multithreaded --maxEvents 300 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--preExec="flags.DQ.Steering.doHLTMon=False" --ignoreErrors 'False' \
--ignorePatterns 'LArRawDataReadingAlg.+ERROR.+Found.+unsupported.+Rod.+block.+type.+0|LArRawDataReadingAlg.+\|.+ERROR.+\|.|ERROR.+message.+limit.+LArRawDataReadingAlg.' \
--outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"
