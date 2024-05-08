#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/DataModelTestRead.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Nov 2005
# Purpose: Test DataVector backwards compatibility.
#          We read old-style DataVectors as new-style,
#          then write them out again new-style.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def DataModelTestReadCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo (CompFactory.DMTest.DMTestRead ('DMTestRead'))

    itemList = ['DataVector<DMTest::B>#bvec',
                'DataVector<DMTest::B>#b3',
                'DMTest::BDer#bder',
                'DataVector<DMTest::D>#dvec',
                'DMTest::DDer#dder',
                'DMTest::ELVec#elvec',
                'DMTest::ELVec#elv_remap']
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList))

    return acc


flags = DataModelTestFlags (infile = 'SimplePoolFile.root',
                            Stream1 = 'SimplePoolFile2.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'DataModelTestRead', loadReadDicts = True)
cfg.merge (DataModelTestReadCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

