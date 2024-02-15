#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IDPVM configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    parser.add_argument("--outputDBFile", help="Name of output DB file", default="beamspot.db", type=string)
    parser.add_argument("--outputHistFile", help="Name of output hist file", default="nt.root", type=string)
    parser.add_argument("--doMonitoring", help="Run monitoring", default=True, type=bool)
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

flags.Trigger.triggerConfig="DB"

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

from InDetBeamSpotFinder.InDetBeamSpotFinderConfig import InDetBeamSpotFinderCfg
acc.merge(InDetBeamSpotFinderCfg(flags))

# Output SQLite and ROOT files
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
acc.merge(IOVDbSvcCfg(
    flags,
    dbConnection= "sqlite://;schema=%s;dbname=BEAMSPOT"
    % MyArgs.outputDBFile))

acc.addService(CompFactory.THistSvc(
    Output = [ "INDETBEAMSPOTFINDER DATAFILE='%s' OPT='RECREATE'"
               % MyArgs.outputHistFile ]))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
