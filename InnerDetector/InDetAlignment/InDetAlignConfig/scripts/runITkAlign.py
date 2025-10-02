#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runIDAlign.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

import os
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles

def parser():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script for the ITk Alignment')
    
    ## Type of running mode
    parser.add_argument("-a", '--accumulate', action="store_true", help='Run accumulation step')
    parser.add_argument("-s", '--solve', action="store_true", help='Run solve step')
    parser.add_argument("-d", '--dryRun', action="store_true", help='Only configure, print and dont execute')
    parser.add_argument("-b", '--baseDir', default = "./", help='Base dir where output is placed')
    
    ## IO
    parser.add_argument("-i", "--input", default = defaultTestFiles.RDO_RUN4, nargs = "+", help='Input file(s)')
    parser.add_argument("--maxEvents", default = -1, type = int, help='Number of maximal processed events')
    parser.add_argument("-t", "--inputTracksCollection", default = "CombinedITkTracks", type = str, help='Name of the track collection to use')
    parser.add_argument("--inputTFiles", default = "AlignmentTFile.root", type = str, help='ROOT file produced in MatrixTool in the accumulation step')
    
    parser.add_argument("--alignmentConstants", default = [], nargs = "+", help='Local alignment constants to use')
    
    ## Things to align
    parser.add_argument("--alignITk", action="store_true", help='Align whole ITk')
    parser.add_argument("--alignITkPixel", action="store_true", help='Align ITkPixel')
    parser.add_argument("--alignITkStrip", action="store_true", help='Align ITkStrip')
    
    ## Tags
    parser.add_argument("--globalTag", default = defaultConditionsTags.RUN4_MC, help='Global tag')
    parser.add_argument("--atlasVersion", default = defaultGeometryTags.RUN4, help='Global tag')
    
    parser.add_argument("--isBFieldOff", action="store_true", help='Check if Bfield is off')
    parser.add_argument("--isCosmics", action="store_true", help='Check if cosmics run')
    parser.add_argument("--isHeavyIon", action="store_true", help='Check if heavy ion run')

    ## Local Geometry
    parser.add_argument("--localgeo", action="store_true", help='Use local geometry XML files')

    ## Local DB File
    parser.add_argument("--localDB", default = "", help='Use local DB file rather than from conditions tag')

    
    return parser.parse_args()

kwargs = vars(parser())

## Create flags and set alignment specific parameter
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

## Disable all non-track related flag parameter
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
OnlyTrackingPreInclude(flags)

## Update flags based on parser line args
flags.ITk.Align.accumulate = kwargs["accumulate"]
flags.ITk.Align.baseDir = os.path.abspath(kwargs["baseDir"])

flags.ITk.Align.alignITk = kwargs["alignITk"] or (not kwargs["alignITk"] and not kwargs["alignITkPixel"] and not kwargs["alignITkStrip"])
flags.ITk.Align.alignITkPixel = kwargs["alignITkPixel"] or flags.ITk.Align.alignITk
flags.ITk.Align.alignITkStrip = kwargs["alignITkStrip"]  or flags.ITk.Align.alignITk

flags.ITk.Align.writeSilicon = False #Issues with folders ATM - should be flags.ITk.Align.alignITkPixel or flags.ITk.Align.alignITkStrip

flags.ITk.Align.inputTFiles = kwargs["inputTFiles"]

flags.Input.Files = kwargs["input"]
flags.Exec.MaxEvents = kwargs["maxEvents"] if not kwargs["solve"] else 1
flags.IOVDb.GlobalTag = kwargs["globalTag"]
    
flags.addFlag("ConstrainedTrackProvider.InputTracksCollection", kwargs["inputTracksCollection"])

flags.GeoModel.Align.Dynamic = False
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
    

if kwargs["localgeo"]:
    flags.ITk.Geometry.AllLocal = True

DBFile = ""
DBName="OFLCOND"
tag="InDetSi_MisalignmentMode_random misalignment"

if kwargs["localDB"]:
    flags.ITk.Align.useLocalDatabase = True
    DBFile = kwargs["localDB"]
    flags.IOVDb.DBConnection ="sqlite://;schema="+DBFile+";dbname="+DBName
    flags.ITk.Geometry.alignmentFolder = "/Indet/AlignITk"

if flags.ITk.Align.alignITkPixel:
    flags.ITk.Geometry.pixelAlignable = True
if flags.ITk.Align.alignITkStrip:
    flags.ITk.Geometry.stripAlignable = True

flags.lock()

from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)

if flags.ITk.Align.useLocalDatabase:
    from IOVDbSvc.IOVDbSvcConfig import addFolders, getSqliteContent
    print("Adding Align Folder "+flags.ITk.Geometry.alignmentFolder+" from local "+DBName+" Database in file "+DBFile)
    cfg.merge(addFolders(flags,flags.ITk.Geometry.alignmentFolder,db=DBName,detDb=DBFile,tag=tag, className="AlignableTransformContainer"))     

from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
cfg.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags))

## Accumulate step
if kwargs["accumulate"] and not kwargs["solve"]:
    os.makedirs(f"{flags.ITk.Align.baseDir}/Accumulate", exist_ok = True)
    os.chdir("Accumulate")
    from InDetAlignConfig.AccumulateITkConfig import ITkAccumulateCfg
    cfg.merge(ITkAccumulateCfg(flags))

## Solve step
elif kwargs["solve"] and not kwargs["accumulate"]:
    os.makedirs(f"{flags.ITk.Align.baseDir}/Solve", exist_ok = True)
    os.chdir("Solve")
    from InDetAlignConfig.SolveITkConfig import ITkSolveCfg
    cfg.merge(ITkSolveCfg(flags))
       
else:
    raise Exception("You can run either the acculumation step or the solve step, but not both or neither at the same time!")

##----- Run the setup -----##
                
if kwargs["dryRun"]:
    cfg.printConfig()
   
else:
    cfg.run()
