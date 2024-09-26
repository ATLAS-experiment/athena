#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestRead3.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of May 2014
# Purpose: Test reading objects with xAOD data.
#          Read output of xAODTestTypelessRead_jo.py.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestRead3Cfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestReadCVec ("xAODTestReadCVec"))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ("xAODTestReadCInfo"))
    acc.addEventAlgo (DMTest.xAODTestRead ("xAODTestRead",
                                           GVecReadKey = ''))
    acc.addEventAlgo (DMTest.xAODTestReadCView ('xAODTestReadCView'))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ("xAODTestReadHVec"))
    acc.addEventAlgo (DMTest.xAODTestReadCVec ("xAODTestReadCVec_copy",
                                               CVecKey = "copy_cvec"))
    acc.addEventAlgo (DMTest.xAODTestReadCInfo ("xAODTestReadCInfo_copy",
                                                CInfoKey = "copy_cinfo"))
    acc.addEventAlgo (DMTest.xAODTestRead ("xAODTestRead_copy",
                                           CTrigReadKey = 'copy_ctrig',
                                           GVecReadKey = '',
                                           CVecWDReadKey = 'copy_cvecWD'))
    acc.addEventAlgo (DMTest.xAODTestReadCView ("xAODTestReadCView_copy",
                                                CViewKey = "copy_cview"))
    acc.addEventAlgo (DMTest.xAODTestReadPVec ("xAODTestReadPVec"))
    acc.addEventAlgo (DMTest.xAODTestReadPVec ("xAODTestReadPVec_copy",
                                               PVecKey = "copy_pvec"))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ("xAODTestReadHVec_copy",
                                               HVecKey = "copy_hvec",
                                               HViewKey = "copy_hview"))

    acc.addEventAlgo (DMTest.xAODTestReadJVec ('xAODTestReadJVec'))
    acc.addEventAlgo (DMTest.xAODTestReadPLinks ('xAODTestReadPLinks'))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata3.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestRead3', loadReadDicts = True)
cfg.merge (xAODTestRead3Cfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

