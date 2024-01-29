#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/ByteStreamTestWrite.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Mar 2016
# Purpose: Test writing objects to bytestream.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def ByteStreamTestWriteCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.DummyDecisionWriter ("DummyDecisionWriter",
                                                  DecisionKey = 'xTrigDecision'))
    acc.addEventAlgo (DMTest.xAODTestWriteCVec ("xAODTestWriteCVec",
                                                CVecKey = 'HLT_DMTest__CVec_cvec'))
    acc.addEventAlgo (DMTest.xAODTestWriteCView ("xAODTestWriteCView",
                                                 CVecKey = 'HLT_DMTest__CVec_cvec',
                                                 CViewKey = 'HLT_DMTest__CView_cview'))
    acc.addEventAlgo (DMTest.xAODTestWriteHVec ("xAODTestWriteHVec",
                                                HVecKey = 'HLT_DMTest__HVec_hvec',
                                                HViewKey = 'HLT_DMTest__HView_hview'))
    acc.addEventAlgo (DMTest.xAODTestDecor ("xAODTestDecor",
                                            ReadPrefix = 'HLT_DMTest__CVec_',
                                            DoCInfo = False,
                                            DoCTrig = False))

    # Making sure that no dyn vars are selected by default.
    acc.addEventAlgo (DMTest.xAODTestWriteCVec ("xAODTestWriteCVec2",
                                                CVecKey = 'HLT_DMTest__CVec_cvec2'))

    bswrite = [ 'DMTest::CVec#cvec.-dVar2.-dtest',
                'DMTest::CView#cview',
                'DMTest::HVec#hvec',
                'DMTest::HView#hview',
                'DMTest::CVec#cvec2' ]
    acc.addEventAlgo (DMTest.HLTResultWriter
                      ("HLTResultWriter",
                       Nav = CompFactory.HLT.Navigation (ClassesToPayload = bswrite,
                                                         ClassesToPreregister = bswrite)))

    return acc


flags = DataModelTestFlags()
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'ByteStreamTestWrite', loadWriteDicts = True)

def WriteBSCfg (flags, itemList):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    event_storage_output = CompFactory.ByteStreamEventStorageOutputSvc(
        OutputDirectory="./",
        AppName="Athena",
        RunNumber=0,
        StreamType = 'EventStorage',
        StreamName = 'StreamBSFileOutput',
        SimpleFileName = 'test.bs',
    )
    acc.addService (event_storage_output)

    bytestream_conversion = CompFactory.ByteStreamCnvSvc(
        name = 'ByteStreamCnvSvc',
        ByteStreamOutputSvcList = [event_storage_output.getName()],
    )
    acc.addService(bytestream_conversion)

    event_info_input = ('xAOD::EventInfo','StoreGateSvc+EventInfo')

    output_stream = CompFactory.AthenaOutputStream(
        name = 'BSOutputStreamAlg',
        EvtConversionSvc = bytestream_conversion.name,
        OutputFile = 'ByteStreamEventStorageOutputSvc',
        ItemList = itemList,
        ExtraInputs = {event_info_input}
    )
    acc.addEventAlgo (output_stream, primary=True)

    acc.addService (CompFactory.StoreGateSvc ('MetaDataStore'))

    return acc

cfg.merge (ByteStreamTestWriteCfg (flags))
cfg.merge (WriteBSCfg (flags, ['HLT::HLTResult#HLTResult_HLT']))


import os
try:
    os.remove('test.bs')
except OSError:
    pass
import os
try:
    os.remove('test.bs.writing')
except OSError:
    pass

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
