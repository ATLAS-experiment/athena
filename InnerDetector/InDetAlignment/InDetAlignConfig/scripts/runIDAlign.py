#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runIDAlign.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

import os

def parser():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script to to the IDAlignment')
    
    ## Type of running mode
    parser.add_argument("-a", '--accumulate', action="store_true", help='Run accumulation step')
    parser.add_argument("-s", '--solve', action="store_true", help='Run solve step')
    parser.add_argument("-d", '--dryRun', action="store_true", help='Only configure, print and dont execute')
    
    ## IO
    parser.add_argument("-i", "--input", default = ["/afs/cern.ch/user/a/atlidali/public/CA_hackathon/InitialDerivation/calibScripts/data23_13p6TeV.00456316.calibration_IDCalib.merge.RAW._lb0541._SFO-ALL._0001.1"], nargs = "+", help='Input file(s)')
    parser.add_argument("--maxEvents", default = -1, type = int, help='Number of maximal processed events')
    parser.add_argument("-t", "--inputTracksCollection", default = "CombinedInDetTracks", type = str, help='Name of the track collection to use')
    
    ## Tags
    parser.add_argument("--globalTag", default = "CONDBR2-BLKPA-2023-03", help='Global tag')
    parser.add_argument("--atlasVersion", default = "ATLAS-R3S-2021-03-00-00", help='Global tag')
    parser.add_argument("--projectName", default = "data23_13p6TeV", help='Global tag')
    
    parser.add_argument("--isBFieldOn", action="store_true", default = True, help='Check if Bfield is non-zero')
    parser.add_argument("--isCosmics", action="store_true", default = False, help='Check if cosmics run')
    parser.add_argument("--isHeavyIon", action="store_true", default = False, help='Check if heavy ion run')
    
    return parser.parse_args()


kwargs = parser()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from InDetAlignConfig.IDAlignFlags import createInDetAlignFlags

flags = initConfigFlags()
flags.addFlagsCategory("InDet.Align", createInDetAlignFlags, prefix=True)
flags.lock()

from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)
    
## Update condition databases
# from InDetAlignConfig.CondConfig import CondCfg
# cfg.merge(CondCfg(flags))

## Accumulate step
if kwargs["accumulate"] and not kwargs["solve"]:        
    os.makedirs("Accumulate", exist_ok = True)
    os.chdir("Accumulate")
    from InDetAlignConfig.AccumulateConfig import AccumulateCfg
    cfg.merge(AccumulateCfg(flags))

## Solve step
elif kwargs["solve"] and not kwargs["accumulate"]:
    os.makedirs("Solve", exist_ok = True)
    os.chdir("Solve")
    from InDetAlignConfig.SolveConfig import SolveCfg
    cfg.merge(SolveCfg(flags))
        
else:
    raise Exception("You can run either the acculumation step or the solve step, but not both at the same time!")
                
##----- Run the setup -----##
                
if kwargs["dryRun"]:
    cfg.printConfig()
   
else:
    cfg.run()

