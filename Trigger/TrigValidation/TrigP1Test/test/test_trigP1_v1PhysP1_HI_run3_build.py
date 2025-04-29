#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Test of HI data 2023 workflow, runs athenaHLT with PhysP1 HI menu
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena

import sys
from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_HI_run3_v1'

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data_hi_2023'
ex.flags = [
    f'Trigger.triggerMenuSetup="{triggermenu}"', 'Trigger.doLVL1=True', 'Trigger.doZDC=True', 'Trigger.doTRT=True',
    'Input.ProjectName="data23_hi"'
]
ex.fpe_auditor = True
ex.max_events = -1
ex.args = '''--postcommand="cfg.getEventAlgo('eTowerMakerFromEfexTowers').NoiseCutsBeginTimestamp = 946684801;cfg.getCondAlgo('jFEXCondAlgo').BeginTimestamp = 946684801;cfg.addPublicTool(CompFactory.LVL1.jFEXFormTOBs('jFEXFormTOBs')).JetEtaCalibrationBeginTimestamp = 946684801;from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags, folder='/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib',tag='', db='sqlite://;schema=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/L1Calo/l1calo_eFEX_HI_NC1p0.db;dbname=CONDBR2'));cfg.merge(addOverride(flags, folder='/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts', tag='', db='sqlite://;schema=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/L1Calo/l1calo_eFEX_HI_NC1p0.db;dbname=CONDBR2'));cfg.merge(addOverride(flags, folder='/TRIGGER/L1Calo/V1/Calibration/JfexNoiseCuts', tag='', db='sqlite://;schema=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/L1Calo/l1calo_jFEX_jets_quantile1.0_pp_met_quantile0.1_HI.db;dbname=CONDBR2'));cfg.merge(addOverride(flags, folder='/TRIGGER/L1Calo/V1/Calibration/JfexModuleSettings', tag='', db='sqlite://;schema=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/L1Calo/l1calo_jFEX_jets_quantile1.0_pp_met_quantile0.1_HI.db;dbname=CONDBR2'));cfg.merge(addOverride(flags, folder='/TRIGGER/L1Calo/V1/Calibration/JfexSystemSettings', tag='', db='sqlite://;schema=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/L1Calo/l1calo_jFEX_jets_quantile1.0_pp_met_quantile0.1_HI.db;dbname=CONDBR2'))"'''

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

sys.exit(test.run())
