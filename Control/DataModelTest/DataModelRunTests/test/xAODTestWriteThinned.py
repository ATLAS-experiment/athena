#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestWriteThinned.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Aug 2019
# Purpose: Test thinning xAOD objects.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestWriteThinnedCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.xAODTestWriteCVec ('xAODTestWriteCVec'))
    acc.addEventAlgo (DMTest.xAODTestWriteCLinks ('xAODTestWriteCLinks'))
    acc.addEventAlgo (DMTest.xAODTestThinCVec ('xAODTestThinCVec', Stream = 'StreamThinned1'))

    # Test combining thinning from two independent algorithms.
    acc.addEventAlgo (DMTest.xAODTestWriteCVec ('xAODTestWriteCVec2',
                                                CVecKey = 'cvec2'))
    acc.addEventAlgo (DMTest.xAODTestThinCVec ('xAODTestThinCVec2a', Stream = 'StreamThinned1',
                                               CVecKey = 'cvec2',
                                               Mask = 5))
    acc.addEventAlgo (DMTest.xAODTestThinCVec ('xAODTestThinCVec2b', Stream = 'StreamThinned1',
                                               CVecKey = 'cvec2',
                                               Mask = 6))


    acc.addEventAlgo (DMTest.xAODTestWriteJVec ('xAODTestWriteJVec'))
    acc.addEventAlgo (DMTest.xAODTestThinJVec ('xAODTestThinJVec',
                                               Stream = 'StreamThinned1',
                                               Mask = 5))

    itemList = [ 'DMTest::CVec#cvec',
                 'DMTest::CAuxContainer#cvecAux.-dVar2.-dtest',
                 'DMTest::CVec#cvec2',
                 'DMTest::CAuxContainer#cvec2Aux.-dVar2.-dtest',
                 'DMTest::CLinks#clinks',
                 'DMTest::CLinksAuxInfo#clinksAux.',
                 'DMTest::CLinksContainer#clinksContainer',
                 'DMTest::CLinksAuxContainer#clinksContainerAux.',
                 'DMTest::CLinksContainer#clinksContainer2',
                 'DMTest::CLinksAuxContainer#clinksContainer2Aux.',
                 'DMTest::CLinksAOD#clinksAOD',
                 'DMTest::JVecContainer#jvecContainer',
                 'DMTest::JVecAuxContainer#jvecContainerAux.',
                ]

    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::CAuxContainer_v1',
                  'DMTest::C_v1',
                  'DMTest::CLinks_v1',
                  'DataVector<DMTest::CLinks_v1>',
                  'DMTest::CLinksAuxInfo_v1',
                  'DMTest::CLinksAuxContainer_v1',
                  'DMTest::JVecContainer_v1',
                  'DataVector<DMTest::JVec_v1>',
                  'DMTest::JVecAuxContainer_v1',
                 ]

    acc.merge (TestOutputCfg (flags, 'Thinned1', itemList, typeNames))

    return acc


flags = DataModelTestFlags (Thinned1 = 'xaodthinned1.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestWriteThinned', loadWriteDicts = True)
cfg.merge (xAODTestWriteThinnedCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

