#!/usr/bin/env python
"""Test CutFlowSvc

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
import sys
from argparse import ArgumentParser

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg, CutFlowOutputList

# Argument parsing
parser = ArgumentParser(prog='test_CutFlowSvc')
parser.add_argument('input', type=str, nargs='?',
                    help='Specify the input file')
parser.add_argument("-d", "--data", default=False,
                    action="store_true", help="Run with data")
parser.add_argument('-t', '--threads', default=0, type=int,
                    help='The number of concurrent threads to run. 0 uses serial Athena.')
parser.add_argument('-p', '--processes', default=0, type=int,
                    help='The number of concurrent processes to run. 0 disables Athena MP.')
parser.add_argument("--sharedWriter", default=False,
                    action="store_true", help="Run with shared writer")
args = parser.parse_args()

maxEvents = 200
flags = initConfigFlags()
if args.input:
  flags.Input.Files = [args.input]
elif args.data:
  flags.Input.Files = defaultTestFiles.RAW_RUN2
  # defaultTestFiles.RAW_RUN2 is now limited to 50 events
  maxEvents = 50
else:
  flags.Input.Files = defaultTestFiles.AOD_RUN2_MC

# Flags relating to multithreaded execution
threads = args.threads
procs = args.processes
flags.Concurrency.NumThreads = threads
flags.Concurrency.NumProcs = procs
if threads > 0:
  maxEvents = max(maxEvents, 10 * threads)
  flags.Scheduler.ShowDataDeps = True
  flags.Scheduler.ShowDataFlow = True
  flags.Scheduler.ShowControlFlow = True
  flags.Concurrency.NumConcurrentEvents = threads
suffix = f"_test_{threads}_{procs}"
if procs > 0:
  maxEvents = max(maxEvents, 10 * procs)
  if args.data:
    suffix += "_data"
  if args.sharedWriter:
    maxEvents *= 5
    flags.MP.UseSharedWriter = True
    suffix += "_sharedWriter"
  flags.MP.WorkerTopDir = f"athenaMP_workers{suffix}"
  flags.MP.EventOrdersFile = f"athenamp_eventorders{suffix}.txt"

flags.Output.AODFileName = f"testAOD{suffix}.pool.root"

flags.lock()

# Setup service
acc = MainServicesCfg(flags)
from AthenaConfiguration.Enums import Format
if flags.Input.Format is Format.BS:
  from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
  acc.merge(ByteStreamReadCfg(flags))
else:
  acc.merge(PoolReadCfg(flags))

# FIXME: this is not resolved properly for RAW data input
if 'EventInfo' not in flags.Input.Collections:
  from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
  acc.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True))

acc.merge(CutFlowSvcCfg(flags))
acc.addEventAlgo(CompFactory.TestFilterReentrantAlg("TestReentrant1", FilterKey="TestReentrant1", Modulo=2))
acc.addEventAlgo(CompFactory.TestFilterReentrantAlg("TestReentrant2", FilterKey="TestReentrant2", Modulo=4))

acc.merge(OutputStreamCfg(flags, "AOD", MetadataItemList=CutFlowOutputList(flags)))

# Debugging
from AthenaCommon.Constants import VERBOSE
acc.getService('CutFlowSvc').OutputLevel = VERBOSE
acc.getPublicTool('BookkeeperTool').OutputLevel = VERBOSE
acc.getEventAlgo('AllExecutedEventsCounterAlg').OutputLevel = VERBOSE

# Execute and finish
sc = acc.run(maxEvents=maxEvents)

# Success should be 0
sys.exit(not sc.isSuccess())
