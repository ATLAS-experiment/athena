#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/CondReadWriteCrest.py
# Date: Mar 2024, modeled after the CondReadWrite.py
# Purpose: Test reading of CREST conditions that are written during runtime (aka Extensible Folders) 
#

from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg

def CondReadWriteCrestCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    ## Setup writer alg that writes new conditions on given LB
    ## --- Keep these lines commented for now. Uncomment when the CREST version of CondWriterExtAlg is implemented
    #    cmds = { 6 : "dmtest_condwriter.py --rs=0 --ls=8  'sqlite://;schema=condtest_rw_test.db;dbname=OFLP200' AttrList_noTag_15 42" }
    #    acc.addEventAlgo (DMTest.CondWriterExtAlg( Commands = cmds ))
    acc.addEventAlgo (DMTest.CondReaderAlg( S2Key = ""))
    acc.addCondAlgo (DMTest.CondAlg1())
    return acc

flags = DataModelTestFlags()
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()

cfg = DataModelTestCfg (flags, 'CondReadWriteCrest',
                        # Increment LBN every two events.
                        EventsPerLB= 2)

# This is how we currently configure the IOV(Db)Svc in the HLT
from AthenaConfiguration.ComponentFactory import CompFactory
cfg.addService (CompFactory.IOVSvc (updateInterval = 'RUN',
                                    forceResetAtBeginRun = False))

from IOVDbSvc.IOVDbSvcConfig import addFolders
cfg.merge (addFolders (flags, '/DMTest/TestAttrList', 'CALO_OFL',
                       tag = 'AttrList_noTag_25',
                       className = 'AthenaAttributeList'))
#                      extensible = True)) ---- eventually we want to make this folder extensible

iovdbsvc = cfg.getService ('IOVDbSvc')
iovdbsvc.Source='CREST'
iovdbsvc.GlobalTag='CREST-Marcelo-2'
iovdbsvc.OutputLevel=2
iovdbsvc.DBInstance=""

iovdbsvc.CacheAlign = 0
iovdbsvc.CacheRun = 0
iovdbsvc.CacheTime = 0

cfg.merge (CondReadWriteCrestCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

