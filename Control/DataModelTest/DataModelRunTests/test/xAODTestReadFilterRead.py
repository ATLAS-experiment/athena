#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestRead3.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Sep 2016
# Purpose: Test reading xAOD objects data with remapping on input.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestReadFilterReadCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec'))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ('xAODTestReadCInfo'))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata_filt.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestReadFilterRead', loadReadDicts = True)
cfg.merge (xAODTestReadFilterReadCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

