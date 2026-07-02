#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena for Run4 with ttbar mu=200
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-input: group.trig-hlt.mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4494_r16635
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

from TriggerTest.MCExecStep import MCGridStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

ex = MCGridStep(
    menu='MC_pp_run4_v1',
    global_tag=defaultConditionsTags.RUN4_MC,
    mc_campaign='Campaigns.PhaseIIPileUp200'
)
ex.input = 'ttbar_pu200_Run4'

ex.flags += [
    'ITk.doTruth=False',
    'Tracking.doTruth=False',
]

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
