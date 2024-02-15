#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform HITS->RDO->RDO_TRIG->AOD
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

hit2aod = ExecStep.ExecStep('HITtoAOD')
hit2aod.type = 'Reco_tf'
hit2aod.input = 'ttbar_HITS'
hit2aod.threads = 1
hit2aod.args = '--inputRDO_BKGFile=' + Input.get_input('pileup_RDO').paths[0]
hit2aod.args += ' --outputAODFile=AOD.pool.root'
hit2aod.args += ' --steering "doOverlay" "doRDO_TRIG"'
hit2aod.args += ' --CA "all:True"'
hit2aod.args += ' --preInclude "all:Campaigns.MC23c"'
hit2aod.args += ' --preExec="flags.Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'"'
hit2aod.args += ' --conditionsTag="default:OFLCOND-MC23-SDR-RUN3-02"'
hit2aod.args += ' --geometryVersion="default:ATLAS-R3S-2021-03-02-00"'
hit2aod.args += ' --autoConfiguration="everything"'
hit2aod.args += ' --postInclude "default:PyJobTransforms.UseFrontier"'
hit2aod.args += ' --jobNumber="1"'
hit2aod.timeout = 5400 # default = 3600 s

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hit2aod]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
