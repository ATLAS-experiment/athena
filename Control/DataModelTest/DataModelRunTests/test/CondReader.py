#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/CondReader.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Jul 2017
# Purpose: Test conditions handling.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def CondReaderCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    #from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
    #acc.merge (AthenaPoolCnvSvcCfg (flags))

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.CondReaderAlg (RLTestKey = '/DMTest/RLTest',
                                            TSTestKey = '/DMTest/TSTest',
                                            S3Key = 'scond3'))
    acc.addCondAlgo (DMTest.CondAlg1())
    acc.addCondAlgo (DMTest.CondAlg2())

    return acc



flags = DataModelTestFlags()
flags.Exec.MaxEvents = 30
# Configure conditions DB output to local sqlite file.
flags.IOVDb.DBConnection = 'sqlite://;schema=condtest.db;dbname=OFLP200'
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.Input.isMC = True
flags.lock()

cfg = DataModelTestCfg (flags, 'CondReader', loadReadDicts = True,
                        # Increment LBN every three events, TS each event.
                        EventsPerLB = 3,
                        TimeStampInterval = 1,
                        readCatalog = 'CondWriter_catalog.xml')

from IOVDbSvc.IOVDbSvcConfig import addFolders
cfg.merge (addFolders (flags, '/DMTest/TestAttrList', 'condtest.db',
                       tag = 'AttrList_noTag',
                       className = 'AthenaAttributeList'))
cfg.merge (addFolders (flags, '/DMTest/S2', 'condtest.db',
                       tag = 'S2_noTag',
                       className = 'DMTest::S2'))
cfg.merge (addFolders (flags, '/DMTest/RLTest', 'condtest.db',
                       tag = 'RL_noTag',
                       className = 'AthenaAttributeList'))
cfg.merge (addFolders (flags, '/DMTest/TSTest', 'condtest.db',
                       tag = 'TS_noTag',
                       className = 'AthenaAttributeList'))

cfg.merge (CondReaderCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
