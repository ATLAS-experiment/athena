#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/DataModelTestWrite.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Nov 2005
# Purpose: Test writing (old-style) DataVector objects.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def DataModelTestWriteCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo (CompFactory.DMTest.DMTestWrite ('DMTestWrite'))

    itemList = ['DataVector<DMTest::B>#bvec',
                'DataVector<DMTest::B>#b3',
                'DMTest::BDer#bder',
                'DataVector<DMTest::D>#dvec',
                'DMTest::DDer#dder',
                'DMTest::ELVec#elvec',
                'DMTest::ELVec#elv_remap']
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList))

    return acc


flags = DataModelTestFlags (Stream1 = 'SimplePoolFile.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'DataModelTestWrite', loadWriteDicts = True)
cfg.merge (DataModelTestWriteCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

