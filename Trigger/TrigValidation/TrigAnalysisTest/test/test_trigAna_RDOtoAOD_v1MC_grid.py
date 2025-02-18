#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD->DAOD with threads=8, MC_pp_run3_v1 and AODSLIM
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
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

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'MC_pp_run3_v1\'',
  'flags.Trigger.AODEDMSet=\'AODSLIM\'',
])

from AthenaConfiguration.TestDefaults import defaultConditionsTags
conditions = defaultConditionsTags.RUN3_MC

rdo2aod = ExecStep.ExecStep()
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar'
rdo2aod.max_events = 800
rdo2aod.threads = 8
rdo2aod.concurrent_events = 8
rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
rdo2aod.args += ' --CA "all:True"'
rdo2aod.args += ' --preExec="all:{:s};"'.format(preExec)
rdo2aod.args += ' --preInclude "all:Campaigns.MC23e"'
rdo2aod.args += ' --conditionsTag "default:' + conditions + '"'

aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = ''
aod2daod.forks = 8
aod2daod.explicit_input = True
aod2daod.args = '--inputAODFile=AOD.pool.root --outputDAODFile=DAOD.pool.root --formats=PHYS'
aod2daod.args += ' --sharedWriter=True --athenaMPMergeTargetSize "DAOD_*:0"'
aod2daod.args += ' --asetup="all:Athena,main,latest"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [rdo2aod, aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
