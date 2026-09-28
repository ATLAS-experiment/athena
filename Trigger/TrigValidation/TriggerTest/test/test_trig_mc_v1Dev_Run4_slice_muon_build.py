#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test for phase-II muon SW in Dev_pp_run4_v1 menu
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

import os
os.environ["PATHRESOLVER_DEVAREARESPONSE"] = "WARNING"

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
from AthenaConfiguration.TestDefaults import defaultConditionsTags

ex = MCBuildStep(menu='Dev_pp_run4_v1', signatures=['Muon'], global_tag=defaultConditionsTags.RUN4_MC, mc_campaign='Campaigns.MC23PhaseIIPileUp200')

ex.input = f'{MuonPhaseIITestDefaults.RDO_R4[0]}'

ex.flags+=[ 'GeoModel.SQLiteDB=True',
           f'GeoModel.SQLiteDBFullPath={MuonPhaseIITestDefaults.GEODB_R4}',
            'Trigger.Offline.SA.Muon.scheduleActsReco=True',
            'ITk.doTruth=False',
            'Tracking.doTruth=False',
            'Acts.TrackingGeometry.UseBlueprint=True',
            'Common.MsgSuppression=False']
ex.imf = False

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = ex.get_log_file_name()
test.check_steps = [check_log, CheckSteps.ChainDumpStep(), CheckSteps.ZeroCountsStep()]

import sys
sys.exit(test.run())
