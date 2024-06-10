#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IDPVM configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    parser.add_argument("--outputFile", help="Name of output file", default="ZdcNtuple.outputs.root", type=str)
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

from ZdcNtuple.ZdcNtupleConfig import ZdcNtupleCfg
acc.merge(ZdcNtupleCfg(flags, name = "AnalysisAlg",
                       zdcConfig = "LHCf2022",
                       lhcf2022 = False,
                       lhcf2022zdc = True,
                       lhcf2022afp = False,
                       zdcOnly = False,
                       useGRL = False,
                       zdcCalib = False,
                       reprocZdc = False,
                       enableOutputTree = True,
                       enableOutputSamples = False,
                       enableTrigger = True,
                       enableTracks = True,
                       enableClusters = True,
                       writeOnlyTriggers = True))

from AthenaConfiguration.ComponentFactory import CompFactory
acc.addService(CompFactory.THistSvc(
    Output = ["ANALYSIS DATAFILE='%s' OPT='RECREATE'" % MyArgs.outputFile]))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
