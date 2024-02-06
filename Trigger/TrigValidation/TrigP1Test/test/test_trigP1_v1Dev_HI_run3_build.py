#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Test of HI data 2023 workflow, runs athenaHLT with HI menu followed by filtering of HP stream and offline reco
# art-type: build
# art-include: master/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Common import find_file


# Specify trigger menu once here:
triggermenu = 'Dev_HI_run3_v1_TriggerValidation_prescale'


hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts/runHLT_standalone.py'
hlt.input = 'data_hi_2023'
hlt.args = f'-c "setMenu=\'{triggermenu}\';doL1Sim=True;'
hlt.args += ';'.join(['flags.Trigger.doZDC=True',
                     'flags.Input.ProjectName=\'data23_hi\'',
                     'flags.Trigger.L1MuonSim.NSWVetoMode=False',
                     'flags.Trigger.L1MuonSim.doMMTrigger=False',
                     'flags.Trigger.L1MuonSim.doPadTrigger=False',
                     'flags.Trigger.L1MuonSim.doStripTrigger=False']) + '"'
hlt.fpe_auditor = True
hlt.max_events = -1
hlt.args += ' -o output'

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
# Tier-0 reco step (BS->AOD)
# see refernce Reconstruction RecExample\RecJobTransformTests\test\test_data22_hi.sh

recoHPPreExec = ';'.join([f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\'", 
                           "flags.Trigger.AODEDMSet=\'AODFULL\'", 
                           "flags.Reco.HIMode=HIMode.HI",
                           "flags.Input.ProjectName='data23_hi'",
                           ])

reco_hp = ExecStep.ExecStep('Tier0RecoHP')
reco_hp.type = 'Reco_tf'
reco_hp.threads = 4
reco_hp.concurrent_events = 4
reco_hp.input = ''
reco_hp.explicit_input = True
reco_hp.max_events = 4
reco_hp.args = '--inputBSFile=' + find_file('*.physics_HardProbes*._athenaHLT*.data')  # output of the previous step
reco_hp.args += ' --outputAODFile=HP_AOD.pool.root'
reco_hp.args += ' --outputHISTFile=hist.root'
reco_hp.args += f' --preExec="all:{recoHPPreExec}"'
reco_hp.args += ' --CA'
reco_hp.args += ' --geometryVersion="ATLAS-R3S-2021-03-02-00"'
reco_hp.args += ' --conditionsTag="CONDBR2-BLKPA-2023-02"'
reco_hp.args += ' --autoConfiguration="everything"'

#====================================================================================================
# Tier-0 UPC reco step (BS->AOD)
# for reference see: Reconstruction/RecExample/RecJobTransformTests/test/test_data22_upc.sh
recoUPCPreExec = ';'.join([f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\'",
                           "flags.Trigger.AODEDMSet=\'AODFULL\'",
                           "flags.Reco.HIMode=HIMode.UPC",
                           "flags.Input.ProjectName='data23_hi'",
                           ])

reco_upc = ExecStep.ExecStep('Tier0RecoUPC')
reco_upc.type = 'Reco_tf'
reco_upc.threads = 4
reco_upc.concurrent_events = 4
reco_upc.input = ''
reco_upc.explicit_input = True
reco_upc.max_events = -1
reco_upc.args = '--inputBSFile=' + find_file('*.physics_UPC*._athenaHLT*.data')  # output of the previous step
reco_upc.args += ' --outputAODFile=AOD_UPC.pool.root'
reco_upc.args += ' --outputHISTFile=hist_UPC.root'
reco_upc.args += f' --preExec="all:{recoUPCPreExec}"'
reco_upc.args += ' --CA'
reco_upc.args += ' --geometryVersion="ATLAS-R3S-2021-03-02-00"'
reco_upc.args += ' --conditionsTag="CONDBR2-BLKPA-2023-02"'
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
