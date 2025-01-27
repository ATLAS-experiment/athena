#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: PhysicsP1_pp_run3_v1 menu test only dumping options for SMK generation and running RuleBook to create prescales
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena

from TrigValTools.TrigValSteering import Step, Test, ExecStep, CheckSteps

#====================================================================================================
# Run Physics menu and produce files to create SMK

menu = ExecStep.ExecStep('RunPhysMenu')
menu.type = 'athenaHLT'
menu.job_options = 'TriggerJobOpts.runHLT'
menu.input = 'data'
menu.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_run3_v1"']
menu.args = '-M --dump-config-exit'
menu.perfmon = False  # Cannot use PerfMon with -M

#====================================================================================================
# Download and setup RuleBook

rbsetup = ExecStep.ExecStep('SetupRulebook')
rbsetup.type = 'other'
rbsetup.executable = 'setupTrigMenuRulebook.sh'
rbsetup.input = ''

#====================================================================================================
# Run RuleBook and create prescales
#
# Needs to source setup script as the environment is not propagated
# from the setupTrigMenuRulebook script

ps = ExecStep.ExecStep('RunRulebook')
ps.type = 'other'
ps.executable = '(source build_rb_test/*/setup.sh && runRuleBook.py PhysicsP1_pp_run3_v1_rules output_RB_ART -o useDefaultBG=True downloadMenu=False BGRP=2515 ignoreErrors=True -l 20000)'
ps.input = ''

#====================================================================================================
# The full test

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [menu,rbsetup,ps]
# Only keep relevant checks from the defaults
test.check_steps = [chk for chk in CheckSteps.default_check_steps(test)
                    if type(chk) in (CheckSteps.LogMergeStep, CheckSteps.CheckLogStep)]

# Ignore expected errors from the rulebook due to mismatch of chains in menu and rules
checklog = test.get_step("CheckLog")
checklog.config_file = 'checklogTrigP1Rulebook.conf'

# Collect error message but do not make the test fail
checkRBerr = CheckSteps.CheckLogStep('CheckRBErrors')
checkRBerr.output_stream = Step.Step.OutputStream.FILE_ONLY
checkRBerr.required = False
test.check_steps.append(checkRBerr)

import sys
sys.exit(test.run())
