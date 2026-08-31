# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigValTools.TrigValSteering import Input, ExecStep, PyStep, Test
from TrigP1Test import TrigP1TestSteps
from contextlib import suppress
import eformat
import json
import os


def test_trigP1_preload(menu):
    """Fully configured _preload test for given menu"""

    # Get run/LB number from input file
    input_file = Input.get_input('data').paths[0]
    first_event = eformat.istream(input_file)[0]
    run = first_event.run_no()
    lb = first_event.lumi_block()

    # 1) Delete any previous bytestream file
    def cleanup():
       with suppress(FileNotFoundError):
          os.remove('raw._0001.data')

    ex_rm = PyStep.PyStep(cleanup)

    # 2) Create new bytestream file with future run number and renumber LBs
    ex_bs = ExecStep.ExecStep('create_bs')
    ex_bs.type = 'other'
    ex_bs.input = ''
    ex_bs.executable = 'trigbs_modifyEvent.py'
    ex_bs.args = '-n 50 --runNumber 999999 --incLB 6 --firstLB 4 --eventsPerLB=3 -o raw %s' % input_file

    # 3) Create configuration JSON based on original data file
    ex_dump = ExecStep.ExecStep('dump-config')
    ex_dump.type = 'athenaEF'
    ex_dump.job_options = 'TriggerJobOpts.runHLT'
    ex_dump.input = 'data'
    ex_dump.args = '-M --dump-config-exit'
    ex_dump.args += f' -C "from TriggerJobOpts import PostExec; PostExec.forceConditions({run},{lb})"'
    ex_dump.flags = [f'Trigger.triggerMenuSetup="{menu}"',
                     'Trigger.doLVL1=True']
    ex_dump.perfmon = False

    # 4) Fix the Json produced in the previous step
    def fix_json():
       # We do not load the PSK from COOL but the folder needs to at least hold some
       # valid data, which is not the case for run 999999. Force it to an arbitrary old run.
       with open('HLTJobOptions.json') as f:
          cfg = json.load(f)

       cfg['properties']['IOVDbSvc']['Folders'] = cfg['properties']['IOVDbSvc']['Folders'].replace(
          '/TRIGGER/HLT/PrescaleKey',f'/TRIGGER/HLT/PrescaleKey<forceRunNumber>{run}</forceRunNumber>')

       with open('HLTJobOptions.fixPS.json','w') as f:
          json.dump(cfg, f)

    ex_fix = PyStep.PyStep(fix_json)

    # 5) Run athenaEF from JSON on renumbered file
    ex = ExecStep.ExecStep('athenaEF')
    ex.type = 'other'
    ex.executable = 'athenaEF.py'
    ex.input = ''
    ex.explicit_input = True
    ex.args = '-M -f ./raw._0001.data'
    ex.args += ' -R 999999 -L 999 --sor-time=now --detector-mask=all'  # to avoid COOL lookup of non-existent run
    ex.args += ' --imf --threads=1 --concurrent-events=1 --nprocs=1'
    ex.args += ' HLTJobOptions.fixPS.json'
    ex.perfmon = False  # Cannot use PerfMon with -M

    # Checks
    test = Test.Test()
    test.art_type = 'build'
    test.exec_steps = [ex_rm, ex_bs, ex_dump, ex_fix, ex]
    test.check_steps = TrigP1TestSteps.default_check_steps_OHMon(test, 'r0000999999_athenaEF_Histogramming.root:run_999999/lb_-1')

    return test
