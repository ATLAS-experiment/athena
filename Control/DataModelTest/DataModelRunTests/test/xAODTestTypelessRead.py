#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestTypelessRead.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of May 2014
# Purpose: Test reading xAOD objects with aux data, w/o compile-time type info.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestTypelessReadCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestTypelessRead ("xAODTestTypelessRead",
                                                   WritePrefix = "copy_"))

    itemList = [ 'DMTest::CVec#cvec',
                 'DMTest::CAuxContainer#cvecAux.',
                 'DMTest::CVecWithData#cvecWD',
                 'DMTest::CView#cview',
                 'DMTest::CAuxContainer#cvecWDAux.',
                 'DMTest::CVec#ctrig',
                 'DMTest::CTrigAuxContainer#ctrigAux.',
                 'DMTest::C#cinfo',
                 'DMTest::CInfoAuxContainer#cinfoAux.',
                 'DMTest::HVec#hvec',
                 'DMTest::HAuxContainer#hvecAux.',
                 'DMTest::HView#hview',
                 'DMTest::CVec#copy_cvec',
                 'DMTest::CAuxContainer#copy_cvecAux.',
                 'DMTest::CVecWithData#copy_cvecWD',
                 'DMTest::CView#copy_cview',
                 'DMTest::CAuxContainer#copy_cvecWDAux.',
                 'DMTest::CVec#copy_ctrig',
                 'DMTest::CTrigAuxContainer#copy_ctrigAux.',
                 'DMTest::C#copy_cinfo',
                 'DMTest::CInfoAuxContainer#copy_cinfoAux.',
                 'DMTest::HVec#copy_hvec',
                 'DMTest::HAuxContainer#copy_hvecAux.',
                 'DMTest::HView#copy_hview' ]
    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::CVecWithData_v1',
                  'DMTest::CAuxContainer_v1',
                  'DMTest::CTrigAuxContainer_v1',
                  'DMTest::C_v1',
                  'DMTest::CInfoAuxContainer_v1' ]

    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames ))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata.root',
                            Stream1 = 'xaoddata3.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestTypelessRead', loadReadDicts = True)
cfg.merge (xAODTestTypelessReadCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

