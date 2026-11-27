#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test for Run4 with ttbar mu=200
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-architecture: '#&nvidia'
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Phase II Pileup 200 MC settings consistent with input file
run = MCBuildStep(
    menu='MC_pp_run4',
    global_tag=defaultConditionsTags.RUN4_MC,
    mc_campaign='Campaigns.MC23PhaseIIPileUp200'
)
run.input = 'ttbar_pu200_Run4'

chains = ["EF_j20_roiftf_preselj20_L1RD0_FILLED", 
          "EF_xe90_pfopufit_L1jXE100",
          "EF_3j60_0eta290_020jvt_bgn277_pf_ftf_presel3j45bgtwo95_L13jJ70p0ETA23",
          #"EF_mu6_idperf_L1MU5VF",  
          #"EF_tau20_idperf_L1eTAU12",
          #"EF_e26_idperf_loose_L1eEM26M"
          ]

run.flags += [
    'Trigger.doRuntimeNaviVal=True',
    'ITk.doTruth=False',
    'Tracking.doTruth=False',
    'Trigger.EFTrackPipeline="G200"',
    'Trigger.useActsTracking=True',
    'Detector.EnableHGTD=False',
    'Tracking.doITkFastTracking=True',
    'Tracking.ITkActsPass.doAthenaToActsCluster=False',
    'Tracking.doPixelDigitalClustering=True',
    'Acts.Device.doClusterization=True',
    'Acts.Device.doSeeding=True',
    'Acts.Device.doTrackReconstruction=True',
    'Acts.TrackingGeometry.BuildDetrayGeometry=True',
    f'Trigger.selectChains="{chains}"',
]

run.max_events = 5

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
