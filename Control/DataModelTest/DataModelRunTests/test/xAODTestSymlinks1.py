#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestSymlinks1.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Apr 2017
# Purpose: Test syminks and hive.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestSymlinks1Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.xAODTestWriteCVec ("xAODTestWriteCVec"))
    acc.addEventAlgo (DMTest.xAODTestWriteCInfo ("xAODTestWriteCInfo"))
    acc.addEventAlgo (DMTest.xAODTestWriteSymlinks ("xAODTestWriteSymlinks"))
    acc.addEventAlgo (DMTest.xAODTestReadSymlink ("xAODTestReadSymlink", Key='cinfo'))

    writeCInfoTool = DMTest.xAODTestWriteCInfoTool ("xAODTestWriteCInfoTool",
                                                    CInfoKey='cinfo2')
    readSymlinkTool = DMTest.xAODTestReadSymlinkTool ("xAODTestReadSymlinkTool",
                                                      Key='cinfo2')
    acc.addEventAlgo (DMTest.xAODTestAlg ("xAODTestAlg",
                                          Tools = [writeCInfoTool,
                                                   readSymlinkTool]))

    return acc


flags = DataModelTestFlags()
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODSymlinks1')
cfg.merge (xAODTestSymlinks1Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

