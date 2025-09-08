#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD with threads=1 and Acts tracking (C100)
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-input: group.trig-hlt.mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583
# art-input-nfiles: 1
# art-athena-mt: 8
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

preExec = ';'.join([
    'flags.Trigger.triggerMenuSetup=\'MC_pp_run4_v1\'',
    'flags.Trigger.AODEDMSet=\'AODFULL\'',
])

rdo2aod = ExecStep.ExecStep()
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar_pu200_Run4'
rdo2aod.threads = 8
rdo2aod.max_events = 100
rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
rdo2aod.args += ' --CA "all:True"'
rdo2aod.args += ' --preExec "all:{:s};"'.format(preExec)
rdo2aod.args += ' --preInclude "all:Campaigns.PhaseIIPileUp200" "RAWtoALL:ActsConfig.ActsCIFlags.actsWorkflowFlags"'
rdo2aod.args += ' --conditionsTag f"default:{defaultConditionsTags.RUN4_MC}"'
rdo2aod.args += ' --ignorePatterns "Propagation.+reached.+the.+step.+count.+limit,Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters"'
rdo2aod.timeout = 5400 # default = 3600 s
rdo2aod.flags = ['Trigger.enabledSignatures=[\'Muon\',\'Egamma\',\'Jet\',\'Bjet\',\'Tau\']',  
                 'Trigger.useActsTracking=True',
                 'Acts.useCache=True',
                 'Acts.GsfRefitActs=True',
                 'Acts.GsfDirectNavigation=True',
                 'Tracking.doPixelDigitalClustering=True',
                 'Tracking.doITkFastTracking=True',
                 'Trigger.doRuntimeNaviVal=True',
                 'ITk.doTruth=False',
                 'Tracking.doTruth=False',
                 f'IOVDb.GlobalTag=\'{defaultConditionsTags.RUN4_MC}\'',
                 ]

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [rdo2aod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
