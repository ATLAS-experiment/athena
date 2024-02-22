#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IDPVM configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    parser.add_argument("--outputDBFile", help="Name of output DB file", default="beamspot.db", type=str)
    parser.add_argument("--outputHistFile", help="Name of output hist file", default="nt.root", type=str)
    parser.add_argument("--doMonitoring", help="Run monitoring", action='store_true')
    parser.add_argument("--outputMonFile", help="Name of output monitoring file",
                        default="beamspotmonitoring.root", type=str)
    return parser.parse_args()

# Parse the arguments
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

flags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    flags.Input.Files += glob(path)

flags.Exec.SkipEvents = MyArgs.skipEvents
flags.Exec.MaxEvents = MyArgs.maxEvents

flags.Trigger.triggerConfig = "DB"
flags.DQ.enableLumiAccess = False
flags.Output.HISTFileName = MyArgs.outputMonFile

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

from InDetBeamSpotFinder.InDetBeamSpotFinderConfig import InDetBeamSpotFinderCfg
acc.merge(InDetBeamSpotFinderCfg(flags))

from AthenaConfiguration.ComponentFactory import CompFactory
acc.addService(CompFactory.THistSvc(
    Output = ["INDETBEAMSPOTFINDER DATAFILE='%s' OPT='RECREATE'" % MyArgs.outputHistFile]))

if MyArgs.doMonitoring:
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "BeamSpotMonitoring")
    from InDetGlobalMonitoringRun3Test.InDetGlobalBeamSpotMonAlgCfg import (
        InDetGlobalBeamSpotMonAlgCfg )
    InDetGlobalBeamSpotMonAlgCfg(helper, acc, flags)
    acc.merge(helper.result())

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
