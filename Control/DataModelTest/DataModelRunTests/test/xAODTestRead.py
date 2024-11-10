#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestRead.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of May 2014
# Purpose: Test reading xAOD object data.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg, rnt


def xAODTestReadCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    is_rntuple, rntkey = rnt (flags)

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec',
                                               WriteKey = 'copy_cvec',
                                               TestDecorSE = not is_rntuple))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ('xAODTestReadCInfo',
                           WriteKey = 'copy_cinfo'))
    acc.addEventAlgo (DMTest.xAODTestRead ('xAODTestRead',
                                           CTrigWriteKey = 'copy_ctrig',
                                           GVecReadKey = rntkey ('gvec'),
                                           GVecWriteKey = rntkey ('copy_gvec'),
                                           CVecWDWriteKey = 'copy_cvecWD'))
    if not is_rntuple:
        acc.addEventAlgo (DMTest.xAODTestReadCView ('xAODTestReadCView',
                                                    WriteKey = 'copy_cview'))
        acc.addEventAlgo (DMTest.xAODTestReadPVec ('xAODTestReadPVec',
                                                   WriteKey = 'copy_pvec'))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ('xAODTestReadHVec',
                                               VecWriteKey = rntkey ('copy_hvec'),
                                               HViewKey = rntkey ('hview'),
                                               ViewWriteKey = rntkey ('copy_hview')))

    acc.addEventAlgo (DMTest.xAODTestDecor ('AuxDataTestDecor1',
                                            DecorName = 'dInt100',
                                            Offset = 100))

    acc.addEventAlgo (DMTest.xAODTestShallowCopy ('xAODTestShallowCopy'))
    acc.addEventAlgo (DMTest.xAODTestShallowCopyHVec ('xAODTestShallowCopyHVec'))

    acc.addEventAlgo (DMTest.xAODTestDecor ('AuxDataTestDecor1_scopy',
                                            ReadPrefix = 'scopy_',
                                            DecorName = 'dInt150',
                                            Offset = 300))

    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadFwdLink',
                                               CVecKey = 'CVecFwdLink',
                                               TestDecorSE = not is_rntuple))

    acc.addEventAlgo (DMTest.xAODTestReadJVec ('xAODTestReadJVec'))
    acc.addEventAlgo (DMTest.xAODTestReadPLinks ('xAODTestReadPLinks'))

    itemList = ['DMTest::CVec#cvec',
                'xAOD::AuxContainerBase!#cvecAux.',
                'DMTest::CVecWithData#cvecWD',
                'DMTest::CView#cview',
                'DMTest::CAuxContainer#cvecWDAux.',
                'DMTest::PVec#pvec',
                'DMTest::PAuxContainer#pvecAux.',
                'DMTest::GVec#gvec',
                'DMTest::GAuxContainer#gvecAux.',
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
                'DMTest::PVec#copy_pvec',
                'DMTest::PAuxContainer#copy_pvecAux.',
                'DMTest::GVec#copy_gvec',
                'DMTest::GAuxContainer#copy_gvecAux.',
                'DMTest::CVec#copy_ctrig',
                'DMTest::CTrigAuxContainer#copy_ctrigAux.',
                'DMTest::C#copy_cinfo',
                'DMTest::CInfoAuxContainer#copy_cinfoAux.',
                'DMTest::HVec#copy_hvec',
                'DMTest::HAuxContainer#copy_hvecAux.',
                'DMTest::HView#copy_hview',

                'DMTest::CVec#scopy_cvec',
                'xAOD::ShallowAuxContainer#scopy_cvecAux.',
                'DMTest::CVecWithData#scopy_cvecWD',
                'xAOD::ShallowAuxContainer#scopy_cvecWDAux.',
                'DMTest::CVec#scopy_ctrig',
                'xAOD::ShallowAuxContainer#scopy_ctrigAux.',
                'DMTest::C#scopy_cinfo',
                'xAOD::ShallowAuxInfo#scopy_cinfoAux.',
                'DMTest::HVec#scopy_hvec',
                'xAOD::ShallowAuxContainer#scopy_hvecAux.',
                'DMTest::JVecContainer#jvecContainer',
                'DMTest::JVecAuxContainer#jvecContainerAux.',
                'DMTest::JVec#jvecInfo',
                'DMTest::JVecAuxInfo#jvecInfoAux.',
                'DMTest::PLinksContainer#plinksContainer',
                'DMTest::PLinksAuxContainer#plinksContainerAux.',
                'DMTest::PLinks#plinksInfo',
                'DMTest::PLinksAuxInfo#plinksInfoAux.',
                ]
    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::CAuxContainer_v1',
                  'DMTest::CVecWithData_v1',
                  'DMTest::CTrigAuxContainer_v1',
                  'DMTest::C_v1',
                  'DMTest::CInfoAuxContainer_v1',
                  'DataVector<DMTest::P_v1>',
                  'DMTest::P_v1',
                  'DMTest::PAuxContainer_v1',
                  'DMTest::JVecContainer_v1',
                  'DataVector<DMTest::JVec_v1>',
                  'DMTest::JVecAuxContainer_v1',
                  'DMTest::JVec_v1',
                  'DMTest::JVecAuxInfo_v1',
                  'DMTest::PLinksContainer_v1',
                  'DataVector<DMTest::PLinks_v1>',
                  'DMTest::PLinksAuxContainer_v1',
                  'DMTest::PLinks_v1',
                  'DMTest::PLinksAuxInfo_v1',
                 ]
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames ))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata.root',
                            Stream1 = 'xaoddata2.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestRead', loadReadDicts = True)
cfg.merge (xAODTestReadCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

