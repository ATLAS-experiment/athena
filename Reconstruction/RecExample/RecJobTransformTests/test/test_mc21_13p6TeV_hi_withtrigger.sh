#!/bin/sh
#
# art-description: Reco_tf runs on MC21 13.6 TeV Heavy Ion using ttbar with HITS input (HI mode)
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8

Reco_tf.py \
--CA "all:True" \
--multithreaded \
--inputHITSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/mc21_13p6TeV/HITSFiles/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/HITS.29625927._000632.pool.root.1 \
--steering 'doRDO_TRIG' \
--maxEvents=20 \
--outputAODFile=myAOD.pool.root  \
--conditionsTag='OFLCOND-MC21-SDR-RUN3-07' \
--postInclude 'all:PyJobTransforms.UseFrontier' \
--preInclude='RAWtoALL:HIRecConfig.HIModeFlags.HImode' \
--preExec='flags.Egamma.doForward=False;flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False;flags.Beam.BunchSpacing=100;flags.Trigger.triggerMenuSetup="Dev_HI_run3_v1";flags.Trigger.AODEDMSet = "AODFULL";flags.Trigger.forceEnableAllChains=True;flags.Trigger.L1.Menu.doHeavyIonTobThresholds=True' \
--autoConfiguration 'everything' \
--postExec='HITtoRDO:cfg.getCondAlgo("TileSamplingFractionCondAlg").G4Version=-1' \
--DataRunNumber '313000'

RES=$?
echo "art-result: $RES Reco"

