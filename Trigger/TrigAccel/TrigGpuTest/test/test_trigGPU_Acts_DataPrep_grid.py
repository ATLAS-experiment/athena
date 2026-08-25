#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger GPU test on data
# art-type: grid
# art-include: main/Athena
# art-input: group.trig-hlt.data25_13p6TeV.00500306.physics_EnhancedBias.merge.RAW
# art-input-nfiles: 1
# art-athena-mt: 8
# art-architecture: '#&nvidia'
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

import os
#Some traccc files still in dev area
os.environ["PATHRESOLVER_DEVAREARESPONSE"] = "WARNING"

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.input = 'ttbar_pu200_Run4'
ex.max_events = 1
ex.threads = 8
ex.job_options = 'ActsGPUDataPreparation/ActsDeviceClusterizationTest.py'
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
# Compare to reference
refcomp = CheckSteps.RegTestStep('RegTest')
refcomp.regex = 'GPU_ActsClusterComparisonAlg.*INFO.*ValSum'
refcomp.reference = 'TrigGpuTest/test_trigGPU_ActsClusterComparison.ref'
refcomp.required = True              # Final exit code depends on this step

test.check_steps = CheckSteps.default_check_steps(test)
test.check_steps.append(refcomp)

import sys
sys.exit(test.run())
