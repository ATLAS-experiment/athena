#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestSymlinks2.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Apr 2017
# Purpose: Test syminks and hive.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestSymlinks2Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.xAODTestReadSymlink ("xAODTestReadSymlink", Key='cinfo'))

    return acc


flags = DataModelTestFlags ('xaoddata.root')
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODSymlinks2', loadReadDicts = True)
cfg.merge (xAODTestSymlinks2Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

