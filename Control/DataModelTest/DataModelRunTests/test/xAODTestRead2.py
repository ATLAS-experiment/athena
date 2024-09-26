#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestRead2.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of May 2014
# Purpose: Test reading xAOD objects.
#          Read output of xAODTestRead.py.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestRead2Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestReadCVec ("xAODTestReadCVec"))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ("xAODTestCInfo"))
    acc.addEventAlgo (DMTest.xAODTestRead ("xAODTestRead"))
    acc.addEventAlgo (DMTest.xAODTestReadCView ('xAODTestReadCView'))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ("xAODTestReadHVec"))
    acc.addEventAlgo (DMTest.xAODTestReadCVec ("xAODTestReadCVec_copy",
                                               CVecKey = "copy_cvec"))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ("xAODTestReadCInfo_copy",
                                                CInfoKey = "copy_cinfo"))
    acc.addEventAlgo (DMTest.xAODTestRead ("xAODTestRead_copy",
                                           CTrigReadKey = 'copy_ctrig',
                                           GVecReadKey = 'copy_gvec',
                                           CVecWDReadKey = 'copy_cvecWD'))
    acc.addEventAlgo (DMTest.xAODTestReadCView ("xAODTestReadCView_copy",
                                                CViewKey = "copy_cview"))
    acc.addEventAlgo (DMTest.xAODTestReadPVec ("xAODTestReadPVec"))
    acc.addEventAlgo (DMTest.xAODTestReadPVec ("xAODTestReadPVec_copy",
                                               PVecKey = "copy_pvec"))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ("xAODTestReadHVec_copy",
                                               HVecKey = "copy_hvec",
                                               HViewKey = "copy_hview"))
    acc.addEventAlgo (DMTest.xAODTestReadCVec ("xAODTestReadCVec_scopy",
                                               CVecKey = "scopy_cvec"))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ("xAODTestReadCInfo_scopy",
                                                CInfoKey = "scopy_cinfo"))
    acc.addEventAlgo (DMTest.xAODTestRead ("xAODTestRead_scopy",
                                           CTrigReadKey = 'scopy_ctrig',
                                           GVecReadKey = '',
                                           CVecWDReadKey = 'scopy_cvecWD'))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ("xAODTestReadHVec_scopy",
                                               HVecKey = "scopy_hvec",
                                               HViewKey = ""))

    acc.addEventAlgo (DMTest.xAODTestReadJVec ('xAODTestReadJVec'))
    acc.addEventAlgo (DMTest.xAODTestReadPLinks ('xAODTestReadPLinks'))

    itemList = [ 'DMTest::CVec#cvec',
                 'xAOD::AuxContainerBase#cvecAux.' ]
    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::C_v1',
                  'DMTest::CAuxContainer_v1' ]
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames ))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata2.root',
                            Stream1 = 'xaoddata2x.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestRead2', loadReadDicts = True)
cfg.merge (xAODTestRead2Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

