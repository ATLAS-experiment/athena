#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/DataModelTestRead2.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Nov 2005
# Purpose: Test DataVector backwards compatibility.
#          Read back new-style DataVectors.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def DataModelTestRead2Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo (CompFactory.DMTest.DMTestRead ('DMTestRead'))

    return acc


flags = DataModelTestFlags (infile = 'SimplePoolFile2.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'DataModelTestRead2', loadReadDicts = True)
cfg.merge (DataModelTestRead2Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

