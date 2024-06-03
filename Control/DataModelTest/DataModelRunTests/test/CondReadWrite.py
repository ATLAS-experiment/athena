#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/CondReadWrite.py
# Author: Frank Winklmeier, scott snyder
# Date: Nov 2023, from old config version of Aug 2018
# Purpose: Test reading of conditions that are written during runtime
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def CondReadWriteCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    ## Setup writer alg that writes new conditions on given LB
    cmds = { 6 : "dmtest_condwriter.py --rs=0 --ls=8  'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 42" }

    acc.addEventAlgo (DMTest.CondWriterExtAlg( Commands = cmds ))
    acc.addEventAlgo (DMTest.CondReaderAlg( S2Key = ""))

    acc.addCondAlgo (DMTest.CondAlg1())

    return acc



flags = DataModelTestFlags()
flags.fillFromArgs()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()


## Cleanup previous file
import os
if os.path.isfile("condtest_rw.db"):
    os.remove("condtest_rw.db")

## Write some initial IOVs and values
os.system("dmtest_condwriter.py --r=0 --ls=0 --lu=4  'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 10")
os.system("dmtest_condwriter.py --rs=0 --ls=5 'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 20")


cfg = DataModelTestCfg (flags, 'CondReadWrite',
                        # Increment LBN every two events.
                        EventsPerLB= 2)

# This is how we currently configure the IOV(Db)Svc in the HLT
from AthenaConfiguration.ComponentFactory import CompFactory
cfg.addService (CompFactory.IOVSvc (updateInterval = 'RUN',
                                    forceResetAtBeginRun = False))

from IOVDbSvc.IOVDbSvcConfig import addFolders
cfg.merge (addFolders (flags, '/DMTest/TestAttrList', 'condtest_rw.db',
                       tag = 'AttrList_noTag',
                       className = 'AthenaAttributeList',
                       extensible = True))
iovdbsvc = cfg.getService ('IOVDbSvc')
iovdbsvc.CacheAlign = 0  # VERY IMPORTANT to get unique queries for folder updates (see Savannah #81092)
iovdbsvc.CacheRun = 0
iovdbsvc.CacheTime = 0

cfg.merge (CondReadWriteCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
