#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of lowMu menu
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-input: group.trig-hlt.valid1.900341.Epos_LHC_minbias_inelastic.recon.RDO.e8514_e8528_s4159_s4114_r14838_tid34209701_00
# art-input-nfiles: 2
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
# art-output: *.check*

from TriggerTest.MCExecStep import MCGridStep
from TrigValTools.TrigValSteering import Test, CheckSteps

ex = MCGridStep(
    menu='PhysicsP1_pp_lowMu_run3_v1',
    mc_campaign='Campaigns.MC23LowMu',
)
ex.input = 'minbias'
# the MC23LowMu campaign is based on MC23a, MC23eLowMu doesn't exist, need to override relevant settings
ex.flags.extend(['Input.MCCampaign=Campaign.MC23e',
                 'Input.ConditionsRunNumber=470000'])

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
