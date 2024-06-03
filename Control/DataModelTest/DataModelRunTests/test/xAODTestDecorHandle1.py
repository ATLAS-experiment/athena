#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestDecorHandle1.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Apr 2017
# Purpose: Test decoration handles and hive.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestDecorHandle1Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.xAODTestWriteCVec ("xAODTestWriteCVec"))
    acc.addEventAlgo (DMTest.xAODTestWriteCInfo ("xAODTestWriteCInfo"))
    acc.addEventAlgo (DMTest.xAODTestWrite ("xAODTestWrite"))
    acc.addEventAlgo (DMTest.xAODTestDecor ("xAODTestDecor"))
    acc.addEventAlgo (DMTest.xAODTestReadDecor ("xAODTestReadDecor"))

    acc.addEventAlgo (DMTest.xAODTestShallowCopy ("xAODTestShallowCopy",
                                                  CVecWDReadKey = '',
                                                  CTrigReadKey = ''))
    acc.addEventAlgo (DMTest.xAODTestReadDecor ("xAODTestReadDecorSCopy",
                                                ReadPrefix = "scopy_"))

    return acc


flags = DataModelTestFlags()
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODDecorHandle1')
cfg.merge (xAODTestDecorHandle1Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

