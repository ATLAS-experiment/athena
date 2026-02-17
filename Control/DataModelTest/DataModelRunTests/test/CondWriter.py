#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/CondWriter.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Jul 2017
# Purpose: Write some conditions objects for testing purposes.
#

#
# For purposes of this test, we assume that timestamp (in sec) matches
# the event number (starting with 0) and that LBN counts every 3 events.
#
# We write four folders:
# /DMTest/TestAttrList (runlbn):
#  Attribute list.  New IOV for every LBN.  xint=(lbn+1)*10
# /DMTest/S2 (runlbn):
#  DMTest::S2.  New IOV for every 2 LBNs.  payload: lbn*50
# /DMTest/RLTest (runlbn):
#  Attribute list, defined as below.
# /DMTest/TSTest (timestamp):
#  Attribute list, defined as below.

#  lbn:     0..1..2..3..4..5..6..7..8..9..
#
# lbn iov:  1..2.....3..4........5..6..7..
#  ts iov:  1..2..34..5......6.7..8...9...   * 100
#
# event:              11111111112222222222
# (ts)      012345678901234567890123456789


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def CondWriterCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
    acc.merge (AthenaPoolCnvSvcCfg (flags))

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    condstream = CompFactory.AthenaOutputStreamTool ('CondStream',
                                                     OutputFile = 'condtest.pool.root',
                                                     TopLevelContainerName = '<type>',
                                                     SubLevelBranchName = '<key>')

    acc.addEventAlgo (DMTest.CondWriterAlg (Streamer = condstream))

    return acc



flags = DataModelTestFlags()
flags.Exec.MaxEvents = 30
# Configure conditions DB output to local sqlite file.
flags.IOVDb.DBConnection = 'sqlite://;schema=condtest.db;dbname=OFLP200'
flags.PoolSvc.DefaultContainerType = 'ROOTTREE'
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'CondWriter', loadWriteDicts = True)

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
cfg.merge (IOVDbSvcCfg (flags))

cfg.merge (CondWriterCfg (flags))

import os
try:
    os.remove('condtest.db')
except OSError:
    pass
try:
    os.remove('condtest.pool.root')
except OSError:
    pass

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

