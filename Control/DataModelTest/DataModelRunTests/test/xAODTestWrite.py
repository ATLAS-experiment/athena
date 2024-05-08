#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestWrite.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of May 2014
# Purpose: Test writing xAOD objects.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestWriteCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestWriteCVec ('xAODTestWriteCVec'))
    acc.addEventAlgo (DMTest.xAODTestWriteHVec ('xAODTestWriteHVec'))
    acc.addEventAlgo (DMTest.xAODTestWriteCView ('xAODTestWriteCView'))
    acc.addEventAlgo (DMTest.xAODTestWriteCInfo ('xAODTestWriteCInfo'))
    acc.addEventAlgo (DMTest.xAODTestWrite ('xAODTestWrite'))
    acc.addEventAlgo (DMTest.xAODTestWriteCVecConst ('xAODTestWriteCVecConst'))
    acc.addEventAlgo (DMTest.xAODTestDecor ('xAODTestDecor'))
    acc.addEventAlgo (DMTest.xAODTestWriteSymlinks ('xAODTestWriteSymlinks'))
    acc.addEventAlgo (DMTest.xAODTestWriteFwdLink1 ('xAODTestWriteFwdLink1'))
    acc.addEventAlgo (DMTest.xAODTestWriteFwdLink2 ('xAODTestWriteFwdLink2'))
    acc.addEventAlgo (DMTest.MetaWriterAlg ('MetaWriterAlg'))

    itemList = ['DMTest::CVec#cvec',
                'DMTest::CAuxContainer#cvecAux.-dVar2.-dtest',
                'DMTest::CVecWithData#cvecWD',
                'DMTest::CView#cview',
                'DMTest::CAuxContainer#cvecWDAux.',
                'DMTest::GVec#gvec',
                'DMTest::GAuxContainer#gvecAux.',
                'DMTest::CVec#ctrig',
                'DMTest::CTrigAuxContainer#ctrigAux.',
                'DMTest::C#cinfo',
                'DMTest::CInfoAuxContainer#cinfoAux.',
                'DMTest::HVec#hvec',
                'DMTest::HAuxContainer#hvecAux.',
                'DMTest::HView#hview',
                'DMTest::S2#S2',
                'DMTest::CVec#CVecFwdLink',
                'DMTest::CAuxContainer#CVecFwdLinkAux.']
    metaItemList = [ 'DMTest::S1#MetaS1',
                     'DMTest::C#MetaC',
                     'DMTest::CInfoAuxContainer#MetaCAux.' ]
    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::CVecWithData_v1',
                  'DMTest::CAuxContainer_v1',
                  'DMTest::CTrigAuxContainer_v1',
                  'ViewVector<DataVector<DMTest::C_v1,DataModel_detail::NoBase> >',
                  'DMTest::C_v1',
                  'DMTest::CInfoAuxContainer_v1',
                  'DataVector<DMTest::G_v1>',
                  'DMTest::GAuxContainer_v1',
                  'DMTest::G_v1',
                  'DMTest::H_v1',
                  'DataVector<DMTest::H_v1>',
                  'ViewVector<DataVector<DMTest::H_v1,DataModel_detail::NoBase> >',
                  'DMTest::HAuxContainer_v1' ]
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames,
                              metaItemList))

    return acc


flags = DataModelTestFlags (Stream1 = 'xaoddata.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestWrite', loadWriteDicts = True,
                        # Increment LBN every two events.
                        EventsPerLB = 2)
cfg.merge (xAODTestWriteCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

