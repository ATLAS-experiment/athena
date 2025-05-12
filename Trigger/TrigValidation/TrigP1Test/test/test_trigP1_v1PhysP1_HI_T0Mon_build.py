#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Test of HI data 2023 workflow, runs athenaHLT with HI menu followed by filtering of HP stream, and offline reco with monitoring
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Common import find_file
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_HI_run3_v1'


hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts.runHLT'
hlt.input = 'data_hi_2023'
hlt.flags = [f'Trigger.triggerMenuSetup="{triggermenu}"',
             'Trigger.doLVL1=True',
             'Trigger.doZDC=True',
             'Trigger.doTRT=True',
             'Input.ProjectName="data23_hi"',
             'Trigger.L1MuonSim.NSWVetoMode=False',
             'Trigger.L1MuonSim.doMMTrigger=False',
             'Trigger.L1MuonSim.doPadTrigger=False',
             'Trigger.L1MuonSim.doStripTrigger=False']
hlt.max_events = 20
hlt.threads = 4
hlt.fpe_auditor = True
hlt.args = '-o output'

#====================================================================================================

# Extract the physics_HardProbes stream out of the BS file with many streams
filter_hp = ExecStep.ExecStep('FilterHP')
filter_hp.type = 'other'
filter_hp.executable = 'trigbs_extractStream.py'
filter_hp.input = ''
filter_hp.args = '-s HardProbes ' + find_file('*_HLTMPPy_output.*.data')

#====================================================================================================

# Extract the physics_UPC stream out of the BS file with many streams
filter_upc = ExecStep.ExecStep('FilterUPC')
filter_upc.type = 'other'
filter_upc.executable = 'trigbs_extractStream.py'
filter_upc.input = ''
filter_upc.args = '-s UPC ' + find_file('*_HLTMPPy_output.*.data')

#====================================================================================================
# Tier-0 reco step (BS->AOD) with offline monitoring
# see refernce Reconstruction RecExample\RecJobTransformTests\test\test_data22_hi.sh

recoHPPreExec = ';'.join([f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\'", 
                           "flags.Trigger.AODEDMSet=\'AODFULL\'", 
                           "flags.Reco.HIMode=HIMode.HI",
                           "flags.Input.ProjectName='data23_hi'",
                           ])

monPreExec = ';'.join([
    "from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff",
    "allSteeringFlagsOff(flags)",
    "flags.DQ.Steering.doDataFlowMon=True",
    "flags.DQ.Steering.doHLTMon=True",
    "flags.DQ.Steering.doLVL1CaloMon=True",
    "flags.DQ.Steering.doGlobalMon=True",
    "flags.DQ.Steering.doLVL1InterfacesMon=True",
    "flags.DQ.Steering.doCTPMon=True",
    "flags.DQ.Steering.HLT.doBjet=True",
    "flags.DQ.Steering.HLT.doInDet=True",
    "flags.DQ.Steering.HLT.doBphys=True",
    "flags.DQ.Steering.HLT.doCalo=True",
    "flags.DQ.Steering.HLT.doEgamma=True",
    "flags.DQ.Steering.HLT.doJet=True",
    "flags.DQ.Steering.HLT.doMET=True",
    "flags.DQ.Steering.HLT.doMinBias=True",
    "flags.DQ.Steering.HLT.doMuon=True",
    "flags.DQ.Steering.HLT.doTau=True",
])

reco_hp = ExecStep.ExecStep('Tier0RecoHP')
reco_hp.type = 'Reco_tf'
reco_hp.threads = 4
reco_hp.input = ''
reco_hp.explicit_input = True
reco_hp.max_events = 4
reco_hp.args = '--inputBSFile=' + find_file('*.physics_HardProbes*._athenaHLT*.data')  # output of the previous step
reco_hp.args += ' --outputAODFile=HP_AOD.pool.root'
reco_hp.args += ' --outputHISTFile=hist.root'
reco_hp.args += f' --preExec="all:{recoHPPreExec}; {monPreExec}"'
reco_hp.args += ' --geometryVersion="ATLAS-R3S-2021-03-02-00"'
reco_hp.args += f' --conditionsTag="{defaultConditionsTags.RUN3_DATA23}"'
reco_hp.args += ' --autoConfiguration="everything"'

#====================================================================================================
# Tier-0 UPC reco step (BS->AOD) with offline monitoring
# for reference see: Reconstruction/RecExample/RecJobTransformTests/test/test_data22_upc.sh
recoUPCPreExec = ';'.join([f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\'",
                           "flags.Trigger.AODEDMSet=\'AODFULL\'",
                           "flags.Reco.HIMode=HIMode.UPC",
                           "flags.Input.ProjectName='data23_hi'",
                           ])

reco_upc = ExecStep.ExecStep('Tier0RecoUPC')
reco_upc.type = 'Reco_tf'
reco_upc.threads = 4
reco_upc.input = ''
reco_upc.explicit_input = True
reco_upc.max_events = -1
reco_upc.args = '--inputBSFile=' + find_file('*.physics_UPC*._athenaHLT*.data')  # output of the previous step
reco_upc.args += ' --outputAODFile=AOD_UPC.pool.root'
reco_upc.args += ' --outputHISTFile=hist_UPC.root'
reco_upc.args += f' --preExec="all:{recoUPCPreExec}; {monPreExec}"'
reco_upc.args += ' --geometryVersion="ATLAS-R3S-2021-03-02-00"'
reco_upc.args += f' --conditionsTag="{defaultConditionsTags.RUN3_DATA23}"'
reco_upc.args += ' --autoConfiguration="everything"'

# The full test
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hlt, filter_hp, filter_upc]  + [reco_hp, reco_upc]

test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
