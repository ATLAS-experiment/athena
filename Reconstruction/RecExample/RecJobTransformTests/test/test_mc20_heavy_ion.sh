#!/bin/sh
#
# art-description: Reco_tf runs on MC20 with HITS input (HI mode). Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN2[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")

Reco_tf.py --CA --multithreaded --maxEvents=20 --autoConfiguration 'everything' --inputHITSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" \
--outputAODFile=AOD.pool.root \
--postInclude 'all:PyJobTransforms.UseFrontier' \
--preInclude='RAWtoALL:HIRecConfig.HIModeFlags.HImode' \
--preExec='flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False;flags.Beam.BunchSpacing=100;' \
--postExec='HITtoRDO:cfg.getCondAlgo("TileSamplingFractionCondAlg").G4Version=-1' \
--DataRunNumber '313000'

RES=$?
echo "art-result: $RES Reco"
