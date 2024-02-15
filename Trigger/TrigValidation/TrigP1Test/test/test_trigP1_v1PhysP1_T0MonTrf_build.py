#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: transform test of BSRDOtoRAW + T0Reco + T0Mon, using v1PhysP1 menu
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale'

#====================================================================================================
# HLT BS_RDO->RAW
hlt = ExecStep.ExecStep('BSRDOtoRAW')
hlt.type = 'Trig_reco_tf'
hlt.forks = 1
hlt.threads = 4
hlt.concurrent_events = 4
hlt.input = 'data'
hlt.max_events = 50
hlt.args = f'--CA --preExec="Trigger.triggerMenuSetup=\'{triggermenu}\' Trigger.doLVL1=True"'
hlt.args += ' --prodSysBSRDO True'
hlt.args += ' --outputBSFile=RAW.pool.root'
hlt.args += ' --outputHIST_HLTMONFile=hltmon.root'
hlt.args += ' --outputDRAW_TRIGCOSTFile=TRIGCOST.pool.root'
hlt.args += ' --outputNTUP_TRIGCOSTFile=cost.ntup.root'
hlt.args += ' --runNumber 440499'  # RunNumber is set by Panda, but ignored by Trf to avoid changes from !48070

#====================================================================================================
# Tier-0 reco step (BS->AOD)
tzrecoPreExec = ' '.join([
  f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';",
  "flags.Trigger.AODEDMSet=\'AODFULL\';",
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
tzreco.type = 'Trig_reco_tf'
tzreco.threads = 4
tzreco.concurrent_events = 4
tzreco.input = ''
tzreco.explicit_input = True
tzreco.max_events = 50
tzreco.args = '--inputBSFile=RAW.pool.root'  # output of the previous step
tzreco.args += ' --outputAODFile=AOD.pool.root'
tzreco.args += ' --outputNTUP_TRIGRATEFile=rate.ntup.root'
tzreco.args += ' --outputHISTFile=hist.root'
tzreco.args += ' --conditionsTag=\'CONDBR2-BLKPA-2022-08\' --geometryVersion=\'ATLAS-R3S-2021-03-00-00\''
tzreco.args += ' --preExec="{:s}"'.format(tzrecoPreExec)
tzreco.args += ' --CA'

#====================================================================================================
# Merging NTUP_TRIGRATE/COST
tzmergecost = ExecStep.ExecStep('Tier0MergeCost')
tzmergecost.type = 'other'
tzmergecost.executable = 'NTUPMerge_tf.py'
tzmergecost.input = ''                                                                             
tzmergecost.args = '--inputNTUP_TRIGCOSTFile="cost.ntup.root" --outputNTUP_TRIGCOST_MRGFile="cost.ntup.merge.root"'

tzmergerate = ExecStep.ExecStep('Tier0MergeRate')
tzmergerate.type = 'other'
tzmergerate.executable = 'NTUPMerge_tf.py'
tzmergerate.input = ''                                                                             
tzmergerate.args = '--inputNTUP_TRIGRATEFile="rate.ntup.root" --outputNTUP_TRIGRATE_MRGFile="rate.ntup.merge.root"'

#====================================================================================================
# The full test
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hlt,tzreco,tzmergecost,tzmergerate]
test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
   step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
