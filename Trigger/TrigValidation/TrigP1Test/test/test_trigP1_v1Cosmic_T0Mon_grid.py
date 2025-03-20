#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Test of cosmic P1+Tier0 workflow, runs athenaHLT with Cosmic_run3_v1 menu followed by offline reco and monitoring
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena
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
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Specify trigger menu once here:
triggermenu = 'Cosmic_run3_v1'

# HLT step (BS->BS)
hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts.runHLT'
hlt.forks = 1
hlt.threads = 8
hlt.concurrent_events = 8
hlt.input = 'data_cos'
hlt.max_events = 2000
hlt.flags = [f'Trigger.triggerMenuSetup="{triggermenu}"',
             'Beam.Type=BeamType.Cosmics',
             'Trigger.forceEnableAllChains=True']
hlt.args = '-o output'

# Extract the physics_Main stream out of the BS file with many streams
filter_bs = ExecStep.ExecStep('FilterBS')
filter_bs.type = 'other'
filter_bs.executable = 'trigbs_extractStream.py'
filter_bs.input = ''
filter_bs.args = '-l 0' # data_cos input includes files from multiple LBs, see ATR-26461
filter_bs.args += ' -s Main ' + find_file('*_HLTMPPy_output.*.data')

# Tier-0 reco step (BS->AOD)
tzrecoPreExec = ' '.join([
  f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';",
  "flags.Trigger.AODEDMSet=\'AODFULL\';",
  "flags.Exec.FPE=500;",
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
  "flags.DQ.Steering.HLT.doJet=False;",
  "flags.DQ.Steering.HLT.doMET=True;",
  "flags.DQ.Steering.HLT.doMinBias=True;",
  "flags.DQ.Steering.HLT.doMuon=True;",
  "flags.DQ.Steering.HLT.doTau=True;",
])

tzreco = ExecStep.ExecStep('Tier0Reco')
tzreco.type = 'Reco_tf'
# TODO: check if we suffer from bad scaling vs number of threads (ATR-29610) when offline monitoring runs in RAWtoALL
tzreco.threads = 4
tzreco.concurrent_events = 4
tzreco.input = ''
tzreco.explicit_input = True
tzreco.max_events = 2000
tzreco.args = '--inputBSFile=' + find_file('*.physics_Main*._athenaHLT*.data')  # output of the previous step
tzreco.args += ' --outputAODFile=AOD.pool.root'
tzreco.args += ' --outputHISTFile=ExampleMonitorOutput.root'
tzreco.args += ' --geometryVersion=\'ATLAS-R3S-2021-03-02-00\''
tzreco.args += f' --conditionsTag="{defaultConditionsTags.RUN3_DATA23}"'
tzreco.args += ' --preExec="{:s}"'.format(tzrecoPreExec)

# The full test
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hlt, filter_bs, tzreco]
test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
