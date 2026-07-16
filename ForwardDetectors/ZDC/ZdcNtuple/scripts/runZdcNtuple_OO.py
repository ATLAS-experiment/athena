#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

#from AthenaCommon.Logging import logging
#from AthenaCommon.Constants import DEBUG, INFO, VERBOSE
from AthenaCommon.Constants import DEBUG

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IDPVM configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--maxEvents", help="Maximum number of events to process", default=-1, type=int)
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
                       zdcConfig = "OONeNe2025",
                       lhcf2022 = False,
                       lhcf2022zdc = False,
                       lhcf2022afp = False,
                       pbpb2023 = False,
                       oo2025 = True,
                       zdcOnly = False,
                       useGRL = True,
                       grlFilename = "$ROOTCOREBIN/data/data25_hi.OxygenOxygen_DetStatus-v137-pro49-01_MERGED_PHYS_HeavyIonP_All_Good_IgnoreBSPOT_INVALID.xml",
                       zdcCalib = False,
                       reprocZdc = True,
                       doZdcCalib = True,
                       auxSuffix = "rerun",
                       enableOutputTree = True,
                       enableOutputSamples = True,
                       enableTrigger = True,
                       enableID = True,
                       enableTracks = True,
                       trackLimit = 1500,
                       enableClusters = True,
                       enableCalo = True,
                       writeOnlyTriggers = True))

from AthenaConfiguration.ComponentFactory import CompFactory
acc.addService(CompFactory.THistSvc(
    Output = ["ANALYSIS DATAFILE='%s' OPT='RECREATE'" % MyArgs.outputFile]))

acc.printConfig(withDetails=True)

acc.foreach_component("*Zdc*").OutputLevel=DEBUG
    
# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
