#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runIDAlign.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

import os
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles

def parser():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script for the IDAlignment')
    
    ## Type of running mode
    parser.add_argument("-a", '--accumulate', action="store_true", help='Run accumulation step')
    parser.add_argument("-s", '--solve', action="store_true", help='Run solve step')
    parser.add_argument("-d", '--dryRun', action="store_true", help='Only configure, print and dont execute')
    parser.add_argument("-b", '--baseDir', default = "./", help='Base dir where output is placed')
    
    ## IO
    parser.add_argument("-i", "--input", default = defaultTestFiles.RAW_RUN3, nargs = "+", help='Input file(s)')
    parser.add_argument("--maxEvents", default = -1, type = int, help='Number of maximal processed events')
    parser.add_argument("-t", "--inputTracksCollection", default = "CombinedInDetTracks", type = str, help='Name of the track collection to use')
    parser.add_argument("--inputTFiles", default = "AlignmentTFile.root", type = str, help='ROOT file produced in MatrixTool in the accumulation step')
    
    parser.add_argument("--alignmentConstants", default = [], nargs = "+", help='Local alignment constants to use')
    parser.add_argument("--bowingDatabase", default = "", help='Local bowing database to use')
    parser.add_argument("--dynamicGlobalDatabase", default = "", help='Local dynamic global database to use')
    
    ## Things to align
    parser.add_argument("--alignInDet", action="store_true", help='Align whole inner detector')
    parser.add_argument("--alignSilicon", action="store_true", help='Align silicon part of the inner detector')
    parser.add_argument("--alignPixel", action="store_true", help='Align pixel')
    parser.add_argument("--alignSCT", action="store_true", help='Align SCT')
    parser.add_argument("--alignTRT", action="store_true", help='Align TRT')
    
    ## Tags
    parser.add_argument("--globalTag", default = defaultConditionsTags.RUN3_DATA, help='Global tag')
    parser.add_argument("--atlasVersion", default = defaultGeometryTags.RUN3, help='Global tag')
    parser.add_argument("--projectName", default = "data23_13p6TeV", help='Global tag')
    
    parser.add_argument("--isBFieldOff", action="store_true", help='Check if Bfield is off')
    parser.add_argument("--isCosmics", action="store_true", help='Check if cosmics run')
    parser.add_argument("--isHeavyIon", action="store_true", help='Check if heavy ion run')
    
    return parser.parse_args()

kwargs = vars(parser())

## Create flags and set alignment specific parameter
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

## Disable all non-track related flag parameter
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
OnlyTrackingPreInclude(flags)

## Update flags based on parser line args
flags.InDet.Align.accumulate = kwargs["accumulate"]
flags.InDet.Align.baseDir = os.path.abspath(kwargs["baseDir"])

flags.InDet.Align.alignInDet = kwargs["alignInDet"]
flags.InDet.Align.alignSilicon = kwargs["alignInDet"] or kwargs["alignSilicon"]
flags.InDet.Align.alignPixel = kwargs["alignInDet"] or kwargs["alignSilicon"] or kwargs["alignPixel"]
flags.InDet.Align.alignSCT = kwargs["alignInDet"]  or kwargs["alignSilicon"] or kwargs["alignSCT"]
flags.InDet.Align.alignTRT = kwargs["alignInDet"] or kwargs["alignTRT"]

flags.InDet.Align.writeSilicon = flags.InDet.Align.alignPixel or flags.InDet.Align.alignSCT
flags.InDet.Align.writeTRT = flags.InDet.Align.alignTRT

flags.InDet.Align.useDynamicAlignFolders = bool(kwargs["dynamicGlobalDatabase"])
flags.InDet.Align.inputAlignmentConstants = kwargs["alignmentConstants"]
flags.InDet.Align.inputBowingDatabase = kwargs["bowingDatabase"]
flags.InDet.Align.inputDynamicGlobalDatabase = kwargs["dynamicGlobalDatabase"]
flags.InDet.Align.inputTFiles = kwargs["inputTFiles"]

flags.Input.Files = kwargs["input"]
flags.Exec.MaxEvents = kwargs["maxEvents"] if not kwargs["solve"] else 1
flags.IOVDb.GlobalTag = kwargs["globalTag"]
    
flags.addFlag("ConstrainedTrackProvider.InputTracksCollection", kwargs["inputTracksCollection"])

flags.GeoModel.Align.Dynamic = True
flags.GeoModel.AtlasVersion = kwargs["atlasVersion"]

if not flags.Input.isMC and kwargs["isCosmics"]:
    from AthenaConfiguration.Enums import BeamType
    
    flags.Beam.NumberOfCollisions = 0
    flags.Beam.Type = BeamType.Cosmics
    flags.Beam.Energy = 0.
    flags.Beam.BunchSpacing = 50

if kwargs["isHeavyIon"]:
    flags.Beam.BunchSpacing = 50
    flags.Reco.EnableHI = True
    flags.HeavyIon.doGlobal = True
      
else:
    flags.Beam.BunchSpacing = 25
            
if not kwargs["isBFieldOff"]:
    flags.BField.solenoidOn = True
    flags.BField.barrelToroidOn = True
    flags.BField.endcapToroidOn = True
        
else:
    flags.BField.solenoidOn = False
    flags.BField.barrelToroidOn = False
    flags.BField.endcapToroidOn = False
    
if not flags.InDet.Align.alignTRT:
    flags.Detector.GeometryTRT = False
    flags.Detector.EnableTRT = False

flags.lock()

from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)

from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
cfg.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags))
    
## Update condition databases
# from InDetAlignConfig.CondConfig import CondCfg
# cfg.merge(CondCfg(flags))

## Accumulate step
if kwargs["accumulate"] and not kwargs["solve"]:
    os.makedirs(f"{flags.InDet.Align.baseDir}/Accumulate", exist_ok = True)
    os.chdir("Accumulate")
    from InDetAlignConfig.AccumulateConfig import AccumulateCfg
    cfg.merge(AccumulateCfg(flags))

## Solve step
elif kwargs["solve"] and not kwargs["accumulate"]:
    os.makedirs(f"{flags.InDet.Align.baseDir}/Solve", exist_ok = True)
    os.chdir("Solve")
    from InDetAlignConfig.SolveConfig import SolveCfg
    cfg.merge(SolveCfg(flags))
           
else:
    raise Exception("You can run either the acculumation step or the solve step, but not both or neither at the same time!")
                
##----- Run the setup -----##
                
if kwargs["dryRun"]:
    cfg.printConfig()
   
else:
    cfg.run()
