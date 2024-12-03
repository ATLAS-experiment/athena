#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Cosmic_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 4
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

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'mc_cosmics'
ex.threads = 4
ex.concurrent_events = 4
ex.args = '-c "setMenu=\'Cosmic_run3_v1\';doCosmics=True;doWriteBS=False;doWriteRDOTrigger=True;from AthenaConfiguration.AllConfigFlags import ConfigFlags;ConfigFlags.Trigger.enableL1CaloPhase1=False;from IOVDbSvc.CondDB import conddb;conddb.addOverride(\'/PIXEL/PixelModuleFeMask\',\'PixelModuleFeMask-SIM-MC16-000-03\');conddb.addOverride(\'/TRT/Calib/PID_NN\', \'TRTCalibPID_NN_v1\');conddb.addOverride(\'/PIXEL/PixelClustering/PixelNNCalibJSON\', \'PixelNNCalibJSON-SIM-RUN2-000-02\')"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
