#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/ByteStreamTestRead.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Mar 2016
# Purpose: Test reading bytestream data.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def ByteStreamTestReadCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.HLTResultReader 
              ("HLTResultReader",
               Nav = CompFactory.HLT.Navigation (ClassesToPayload = [],
                                                 ClassesToPreregister = [])))

    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec',
                                               CVecKey = 'HLT_DMTest__CVec_cvec'))
    acc.addEventAlgo (DMTest.xAODTestReadCView ('xAODTestReadCView',
                                                CViewKey = 'HLT_DMTest__CView_cview'))
    acc.addEventAlgo (DMTest.xAODTestReadHVec ('xAODTestReadHVec',
                                               HVecKey = 'HLT_DMTest__HVec_hvec',
                                               HViewKey = 'HLT_DMTest__HView_hview'))
    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec2',
                                               CVecKey = 'HLT_DMTest__CVec_cvec2'))

    return acc


flags = DataModelTestFlags()
flags.Input.Files = [ 'test.bs' ]
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'ByteStreamTestRead', loadReadDicts = True)

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
cfg.merge (ByteStreamReadCfg (flags, ['HLT::HLTResult/HLTResult_HLT']))

cfg.merge (ByteStreamTestReadCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
