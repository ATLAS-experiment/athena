#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Test of HLT reproc workflow, runs athenaEF with Dev_pp_run4_v1 menu followed by offline reco and monitoring (incl. EDM)
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Specify trigger menu once here:
triggermenu = 'Dev_pp_run4_v1_HLTReprocessing_prescale'

# HLT BS_RDO->RAW
hlt = ExecStep.ExecStep('BSRDOtoRAW')
hlt.type = 'Trig_reco_tf'
hlt.threads = 4
hlt.concurrent_events = 4
hlt.input = 'data'
hlt.max_events = 50
hlt.args = f'--preExec="Trigger.triggerMenuSetup=\'{triggermenu}\' Trigger.doLVL1=True Trigger.EDMVersion=4 Trigger.CTP.UseEDMxAOD=False Trigger.CTP.UseRoibROB=True"'
hlt.args += ' --prodSysBSRDO True'
hlt.args += ' --outputBSFile=RAW.pool.root'
hlt.args += ' --outputHIST_HLTMONFile=hltmon.root'
hlt.args += ' --outputDRAW_TRIGCOSTFile=TRIGCOST.pool.root'
hlt.args += ' --outputNTUP_TRIGCOSTFile=cost.ntup.root'
hlt.args += ' --runNumber 500306'  # RunNumber is set by Panda, but ignored by Trf to avoid changes from !48070

# BS->AOD reco step
recoPreExec = ' '.join([
  f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';",
  "flags.Trigger.AODEDMSet=\'AODFULL\';",
  "flags.Trigger.EDMVersion=4;",
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
  "flags.Trigger.CTP.UseEDMxAOD=False;",
  "flags.Trigger.CTP.UseRoibROB=True;",
])

reco = ExecStep.ExecStep('Tier0Reco')
reco.type = 'Trig_reco_tf'
reco.threads = 4
reco.concurrent_events = 4
reco.input = ''
reco.explicit_input = True
reco.max_events = 50
reco.args = '--inputBSFile=RAW.pool.root'  # output of the previous step
reco.args += ' --outputAODFile=AOD.pool.root'
reco.args += ' --outputNTUP_TRIGRATEFile=rate.ntup.root'
reco.args += ' --outputHISTFile=ExampleMonitorOutput.root'
reco.args += f' --conditionsTag="{defaultConditionsTags.RUN3_DATA25}" --geometryVersion=\'ATLAS-R3S-2021-03-02-00\''
reco.args += ' --preExec="{:s}"'.format(recoPreExec)

#====================================================================================================
# Merging NTUP_TRIGRATE/COST
recomergecost = ExecStep.ExecStep('Tier0MergeCost')
recomergecost.type = 'other'
recomergecost.executable = 'NTUPMerge_tf.py'
recomergecost.input = ''
recomergecost.args = '--inputNTUP_TRIGCOSTFile="cost.ntup.root" --outputNTUP_TRIGCOST_MRGFile="cost.ntup.merge.root"'

recomergerate = ExecStep.ExecStep('Tier0MergeRate')
recomergerate.type = 'other'
recomergerate.executable = 'NTUPMerge_tf.py'
recomergerate.input = ''
recomergerate.args = '--inputNTUP_TRIGRATEFile="rate.ntup.root" --outputNTUP_TRIGRATE_MRGFile="rate.ntup.merge.root"'

#====================================================================================================

# The full test
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hlt, reco, recomergecost, recomergerate]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
