#!/bin/sh
#
# art-description: Reco_tf runs on MC23a 13.6 TeV Heavy Ion using ttbar with HITS input (HI mode). Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

# TODO update following ATLASRECTS-8054
export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN3_HI[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Reco_tf.py --CA "all:True" --multithreaded --maxEvents=20 --autoConfiguration 'everything' \
--inputHITSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" \
--steering 'doRDO_TRIG' \
--outputAODFile=myAOD.pool.root  \
--preInclude 'RAWtoALL:HIRecConfig.HIModeFlags.HImode' 'all:Campaigns.MC23a' \
--postInclude 'all:PyJobTransforms.UseFrontier' \
--preExec 'flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False;flags.Beam.BunchSpacing=100;flags.Trigger.triggerMenuSetup="Dev_HI_run3_v1";flags.Trigger.AODEDMSet = "AODFULL";flags.Trigger.forceEnableAllChains=True;flags.Trigger.L1.Menu.doHeavyIonTobThresholds=True' \
--postExec 'HITtoRDO:cfg.getCondAlgo("TileSamplingFractionCondAlg").G4Version=-1'

RES=$?
echo "art-result: $RES Reco"
