#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for PhysValExample configuration')
    parser.add_argument("--filesInput")
    parser.add_argument("--outputFile", help='Name of output file', default="M_output.root")
    return parser.parse_args()

# Parse the arguments
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    flags.Input.Files += glob(path)
flags.PhysVal.OutputFileName = MyArgs.outputFile

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

flags.lock()

from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg, PhysValExampleCfg
acc.merge(PhysValMonitoringCfg(flags, tools=[acc.popToolsAndMerge(PhysValExampleCfg(flags))]))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run(maxEvents=-1)

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
