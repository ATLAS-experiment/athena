#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/CondReadWrite.py
# Author: Frank Winklmeier, scott snyder, Marcelo Vogel
# Date: April 2025, from old config version of Aug 2018
# Purpose: Test reading of conditions that are written during runtime
#
import argparse
import tempfile

from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaCommon.Constants import INFO

def CondReadWriteCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    ## Setup writer alg that writes new conditions on given LB

    if args.condDB == 'cool':
        cmds = { 6 : "dmtest_condwriter.py --rs=0 --ls=8  'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 42" }
    else:
        cmds = { 6 : f"dmtest_condwriter.py --rs=0 --ls=8 --tag=Test_AttrList_noTag --xint=42 --gtag=TEST-HLT-CREST --label='/DMTest/TestAttrList' --host={cond_src}" }

    acc.addEventAlgo (DMTest.CondWriterExtAlg( Commands = cmds ))
    acc.addEventAlgo (DMTest.CondReaderAlg( S2Key = ""))

    acc.addCondAlgo (DMTest.CondAlg1())

    return acc

flags = initConfigFlags()
flags.Exec.MaxEvents = 20
flags.Common.MsgSourceLength = 18

# Disable FPE auditing.
flags.Exec.FPE = -2

# Block input file peeking.
from Campaigns.Utils import Campaign
flags.Input.RunNumbers = [0]
flags.Input.TimeStamps = [0]
flags.Input.ProcessingTags = []
flags.Input.TypedCollections = []
flags.Input.isMC = True
flags.IOVDb.GlobalTag = ''
flags.Input.MCCampaign = Campaign.Unknown

parser = flags.getArgumentParser()
parser.add_argument('--condDB', default='cool', help='source of conditions data')
args, _ = parser.parse_known_args()
args, leftovers = parser.parse_known_args()
if flags.Concurrency.NumThreads >= 1:
    flags.Scheduler.ShowDataDeps = True
flags.lock()

## Cleanup previous file
import os, shutil
if os.path.isfile("condtest_rw.db"):
    os.remove("condtest_rw.db")

## Check if CREST_URL is set, fail otherwise
crest_url = os.getenv("CREST_URL")
if not crest_url:
    print("ERROR: Environment variable CREST_URL is not set.\n")
    sys.exit(1)

if args.condDB == 'cool':
    condtest_rw = 'condtest_rw.db'
elif args.condDB == 'crest':
    condtest_rw = 'CALO_OFL'
    cond_src = crest_url
elif args.condDB == 'crestfs':
    tmp_crestfs=tempfile.mkdtemp()
    condtest_rw = tmp_crestfs
    cond_src = tmp_crestfs

## Write some initial IOVs and values
if args.condDB == 'cool':
    os.system("dmtest_condwriter.py --r=0 --ls=0 --lu=4  'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 10")
    os.system("dmtest_condwriter.py --rs=0 --ls=5 'sqlite://;schema=condtest_rw.db;dbname=OFLP200' AttrList_noTag 20")
else:
    os.system(f"dmtest_condwriter.py --r=0 --ls=0 --tag=Test_AttrList_noTag --xint=10 --gtag=TEST-HLT-CREST --label='/DMTest/TestAttrList' --host={cond_src}")
    os.system(f"dmtest_condwriter.py --rs=0 --ls=5 --tag=Test_AttrList_noTag --xint=20 --gtag=TEST-HLT-CREST --label='/DMTest/TestAttrList' --host={cond_src}")

cfg = DataModelTestCfg (flags, 'CondReadWrite',
                        # Increment LBN every two events.
                        EventsPerLB= 2)

# This is how we currently configure the IOV(Db)Svc in the HLT
from AthenaConfiguration.ComponentFactory import CompFactory
cfg.addService (CompFactory.IOVSvc (updateInterval = 'RUN',
                                    forceResetAtBeginRun = False))

from IOVDbSvc.IOVDbSvcConfig import addFolders
cfg.merge (addFolders (flags, '/DMTest/TestAttrList', condtest_rw,
                       tag = 'AttrList_noTag',
                       className = 'AthenaAttributeList',
                       extensible = True))
iovdbsvc = cfg.getService ('IOVDbSvc')
if args.condDB.startswith('crest'):
    iovdbsvc.Source='CREST'
    iovdbsvc.GlobalTag='TEST-HLT-CREST'
    iovdbsvc.crestServer=cond_src

iovdbsvc.CacheAlign = 0  # VERY IMPORTANT to get unique queries for folder updates (see Savannah #81092)
iovdbsvc.CacheRun = 0
iovdbsvc.CacheTime = 0

cfg.merge (CondReadWriteCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)

# Remove tag and global tag map from crest server before exiting
if args.condDB == 'crest':
    os.system(f"dmtest_condwriter.py --remove --tag=Test_AttrList_noTag --gtag=TEST-HLT-CREST --label='/DMTest/TestAttrList' --host={cond_src}")
# Remove crest file sytem storage before exiting
if args.condDB == 'crestfs' and os.path.isdir(tmp_crestfs):
    shutil.rmtree(tmp_crestfs)

import sys
sys.exit (sc.isFailure())
