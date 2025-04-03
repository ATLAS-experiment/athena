#!/bin/sh
#
# art-description: Reco_tf runs on 2011 Heavy Ion data with HardProbes stream. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=4
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN1_DATA11_HI[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN1_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN1_2011)")

Reco_tf.py --CA --multithreaded --maxEvents 10 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--autoConfiguration 'everything' \
--outputAODFile myAOD.pool.root \
--preInclude "all:HIRecConfig.HIModeFlags.HImode" \
--preExec "all:flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False" \
--postExec 'all:from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags,"/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1"));cfg.merge(addOverride(flags,"/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v1"));cfg.getCondAlgo("LArADC2MeVCondAlg").CompleteDetector=False' \

RES=$?
echo "art-result: $RES Reco"
