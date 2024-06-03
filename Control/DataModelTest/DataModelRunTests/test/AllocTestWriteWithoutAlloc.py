#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/AllocTestWriteWithoutAlloc.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Nov 2022
# Purpose: Testing an xAOD object with a non-standard memory allocator.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def AllocTestWriteWithoutAlloc (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.AllocTestWriteWithoutAlloc ('AllocTestWriteWithoutAlloc'))

    itemList = [ 'DMTest::AllocTestContainer#AllocTest',
                 'DMTest::AllocTestAuxContainer#AllocTestAux.',
                 'xAOD::EventInfo#EventInfo',
                 'xAOD::EventAuxInfo#EventInfoAux.' ]

    typeNames = [ 'DataVector<DMTest::AllocTest_v1>',
                  'DMTest::AllocTest_v1',
                  'DMTest::AllocTestAuxContainer_v1' ]

    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames))

    return acc


flags = DataModelTestFlags (Stream1 = 'alloctestWithoutAlloc.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'AllocTestWriteWithoutAlloc', loadWriteDicts = True)
cfg.merge (AllocTestWriteWithoutAlloc (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

