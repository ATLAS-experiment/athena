#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Test of P1+Tier0 workflow, runs athenaHLT with PhysicsP1_pp_run3_v1 menu followed by offline reco, DAOD production, monitoring and analysis step for EDM monitoring
# art-type: grid
# art-athena-mt: 4
# art-include: main/Athena
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Common import find_file
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale'

# HLT step (BS->BS)
hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts/runHLT_standalone.py'
hlt.forks = 1
hlt.threads = 4
hlt.concurrent_events = 4
hlt.input = 'data'
hlt.args = f'-c "setMenu=\'{triggermenu}\';doL1Sim=True;"'
hlt.args += ' -o output'
hlt.args += ' --dump-config-reload'

# Extract the physics_Main stream out of the BS file with many streams
filter_bs = ExecStep.ExecStep('FilterBS')
filter_bs.type = 'other'
filter_bs.executable = 'trigbs_extractStream.py'
filter_bs.input = ''
filter_bs.args = '-s Main ' + find_file('*_HLTMPPy_output.*.data')

# Tier-0 reco step (BS->AOD)
tzrecoPreExec = ' '.join([
  f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';",
  "flags.Trigger.AODEDMSet=\'AODFULL\';",
  "from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff;",
  "allSteeringFlagsOff(flags);",
  "flags.DQ.Steering.doDataFlowMon=True;",
  "flags.DQ.Steering.doHLTMon=True;",
  "flags.DQ.Steering.doLVL1CaloMon=True;",
  "flags.DQ.Steering.doGlobalMon=True;",
  "flags.DQ.Steering.doLVL1InterfacesMon=True;",
  "flags.DQ.Steering.doCTPMon=True;",
  "flags.DQ.Steering.HLT.doBjet=True;",
  "flags.DQ.Steering.HLT.doInDet=True;",
  "flags.DQ.Steering.HLT.doBphys=True;",
  "flags.DQ.Steering.HLT.doCalo=True;",
  "flags.DQ.Steering.HLT.doEgamma=True;",
  "flags.DQ.Steering.HLT.doJet=True;",
  "flags.DQ.Steering.HLT.doMET=True;",
  "flags.DQ.Steering.HLT.doMinBias=True;",
  "flags.DQ.Steering.HLT.doMuon=True;",
  "flags.DQ.Steering.HLT.doTau=True;",
])

tzreco = ExecStep.ExecStep('Tier0Reco')
tzreco.type = 'Reco_tf'
tzreco.threads = 4
tzreco.concurrent_events = 4
tzreco.input = ''
tzreco.explicit_input = True
tzreco.args = '--inputBSFile=' + find_file('*.physics_Main*._athenaHLT*.data')  # output of the previous step
tzreco.args += ' --outputAODFile=AOD.pool.root'
tzreco.args += ' --outputHISTFile=hist.root'
tzreco.args += ' --conditionsTag=\'CONDBR2-BLKPA-2022-08\' --geometryVersion=\'ATLAS-R3S-2021-03-00-00\''
tzreco.args += ' --preExec="{:s}"'.format(tzrecoPreExec)
tzreco.args += ' --CA'

aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = ''
aod2daod.forks = 4
aod2daod.explicit_input = True
aod2daod.args = '--inputAODFile=AOD.pool.root --outputDAODFile=DAOD.pool.root --CA --formats=PHYS'
aod2daod.args += ' --sharedWriter=True --athenaMPMergeTargetSize "DAOD_*:0"'

# The full test
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hlt, filter_bs, tzreco, aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
