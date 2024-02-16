#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: test reloading of conditions during stop/start
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

# This test simulates conditions changes between runs, i.e. between stop/start
# transitions like it would happen at P1. In order to be able to increment the run
# number we employ a rather nasty trick:
#   1) the first run is done on the regular input file symlinked to raw.data
#   2) after stop, we switch the symlink to a copy of the file with the
#      run number and timestamp increased
#
# The files are prepared here, and the symlinking and writing of conditions
# happens in Testing/condStopStart.trans. The expected outcome is:
#   1st run:  xint = 10 (Run/LB IOV) and 100 (Timebased IOV)
#   2nd run:  xint = 20 (Run/LB IOV) and 200 (Timebased IOV)
#

from TrigValTools.TrigValSteering import Test, Step, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Input import get_input

import os
import eformat

# Create a symlink to the first input file
input_file = get_input('data').paths[0]
first_event = eformat.istream(input_file)[0]
run_no = first_event.run_no()

try:
   os.remove('raw.data')
except Exception:
   pass
os.symlink(input_file, 'raw.data')

# Delete any previous bytestream file
ex_rm = ExecStep.ExecStep('cleanup')
ex_rm.type = 'other'
ex_rm.input = ''
ex_rm.executable = 'rm'
ex_rm.args = '-f cond*.data'
ex_rm.auto_report_result = False  # Do not set art-result for this step
ex_rm.output_stream = Step.Step.OutputStream.STDOUT_ONLY  # Do not create a log file for this step

# Make a copy of input BS file with increased run number and timestamp
t_future = 2000000000  # in the future, needs to match condStopStart.trans
ex_bs = ExecStep.ExecStep('create_bs')
ex_bs.type = 'other'
ex_bs.input = ''
ex_bs.executable = 'trigbs_modifyEvent.py'
ex_bs.args = f'-r {run_no+1} -t {t_future} -n 5 -o cond {input_file}'

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TrigP1Test.CondReader.run'
ex.input = ''
ex.max_events = 5
ex.args = '-f raw.data -i -M -ul'
ex.perfmon = False

# Pass the transitions file into athenaHLT -i
ex.cmd_suffix = ' < `find_data.py condStopStart.trans`'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_rm, ex_bs, ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Extra merging pattern for logs produced with -ul option
logmerge = test.get_step("LogMerge")
logmerge.extra_log_regex = 'athenaHLT-.*-.*(.out|.err)'

# Compare to reference
refcomp = CheckSteps.RegTestStep('RegTest')
refcomp.regex = 'CondReaderAlg.*xint'
refcomp.reference = 'TrigP1Test/test_trigP1_condStopStart.ref'
refcomp.required = True              # Final exit code depends on this step
test.check_steps.insert(-1, refcomp) # Add before the last (zip) step

import sys
sys.exit(test.run())
