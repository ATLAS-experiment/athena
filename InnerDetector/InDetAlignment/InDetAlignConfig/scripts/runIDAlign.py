#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runIDAlign.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

import contextlib
import os
import AthenaCommon.Constants
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles

def parser():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script for the IDAlignment')
    
    ## Type of running mode
    parser.add_argument("-a", '--accumulate', action="store_true", help='Run accumulation step')
    parser.add_argument("-s", '--solve', action="store_true", help='Run solve step')
    parser.add_argument("-d", '--dryRun', action="store_true", help='Only configure, print and dont execute')
    parser.add_argument("-b", '--baseDir', default = "./", help='Base dir where output is placed')
    
    ## Input file/track setup for accumulate file
    parser.add_argument("-i", "--input", default = defaultTestFiles.RAW_RUN3_DATA24, nargs = "+", help='Input file(s)')
    parser.add_argument("--maxEvents", default = -1, type = int, help='Number of maximal processed events')
    parser.add_argument("-t", "--inputTracksCollection", default = "CombinedInDetTracks", type = str, help='Name of the track collection to use')
    parser.add_argument("--logLevel", default = "INFO", type = str, help='Log level for messages')
    
    ## Output files created in Accumulate/Solve step
    parser.add_argument("--inputTFiles", nargs = "+", default = [], help='ROOT file produced in MatrixTool in the accumulation step')
    parser.add_argument("--monitorFile", type = str, default = "", help = 'Name of monitor file to output, if wished')
    parser.add_argument("--outputConditionFile", type = str, default = "alignment_output.pool.root", help = "Output POOL file with constants created in solve step")
    parser.add_argument("--outputDBFile", type = str, default = "alignment_output.db", help = "Output database file with constants created in solve step")
    
    ## Local database file to use after initial iteration
    parser.add_argument("--bowingDatabase", default = "", help='Local bowing database to use')
    parser.add_argument("--localDatabase", default = "", help='Local database to use')
    
    ## Alignment setup
    parser.add_argument("--alignLevel", type = int, default = 11, choices = [11, 16, 2, 3], help = "Set alignment level")
    parser.add_argument("--excludeIDPart", nargs = "+", default = [], choices = ["Pixel", "SCT", "TRT"], help = "By default all ID is aligned, exclude some if wished (which may conflict with the set alignment level)")
    
    ## Tags
    parser.add_argument("--globalTag", default = defaultConditionsTags.RUN3_DATA, help='Global tag')
    parser.add_argument("--beamSpotTag", default = "", help='Tag to update')
    parser.add_argument("--L1IDTag", default = "", help='Tag to update')
    parser.add_argument("--L2PIXTag", default = "", help='Tag to update')
    parser.add_argument("--L2SCTTag", default = "", help='Tag to update')
    parser.add_argument("--L1TRTTag", default = "", help='Tag to update')
    parser.add_argument("--L3SiTag", default = "", help='Tag to update')
    parser.add_argument("--L2TRTTag", default = "", help='Tag to update')
    parser.add_argument("--L3TRTTag", default = "", help='Tag to update')
    parser.add_argument("--errorScalingTag", default = "", help='Tag to update')
    parser.add_argument("--lorentzAngleTag", default = "", help='Tag to update')
    parser.add_argument("--MDNTag", default = "", help='Tag to update')
    parser.add_argument("--pixelDistortionTag", default = "", help='Tag to update')
    parser.add_argument("--TRTCalibT0TagCos", default = "", help='Tag to update')
    parser.add_argument("--TRTCalibRtTagCos", default = "", help='Tag to update')
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

## Turn off ID parts if wished (may cause conflicts with level setting)
for IDpart in kwargs["excludeIDPart"]:
    setattr(flags.InDet.Align, f"align{IDpart}", False)

## Set Tags
for tag, tagValue in {tag: value for (tag, value) in kwargs.items() if "Tag" in tag and tag != "globalTag"}.items():
    setattr(flags.InDet.Align, tag, tagValue)

## Set configuration for chosen alignment level
from InDetAlignConfig.IDAlignFlags import setL11AlignmentFlags, setL16AlignmentFlags, setL2AlignmentFlags, setL3AlignmentFlags

localDataBase = os.path.abspath(kwargs["localDatabase"]) if kwargs["localDatabase"] else ""

if kwargs["alignLevel"] == 11:
    setL11AlignmentFlags(flags, localDataBase)
   
elif kwargs["alignLevel"] == 16:
    setL16AlignmentFlags(flags, localDataBase)
    
elif kwargs["alignLevel"] == 2:
    setL2AlignmentFlags(flags, localDataBase)
    
elif kwargs["alignLevel"] == 3:
    setL3AlignmentFlags(flags, localDataBase)

else:
    raise Exception(f"No valid alignment level has been selected: '{kwargs['alignLevel']}'")

## Disable all non-track related flag parameter
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
OnlyTrackingPreInclude(flags)

## Update flags based on parser line args
flags.InDet.Align.accumulate = kwargs["accumulate"]
flags.InDet.Align.baseDir = os.path.abspath(kwargs["baseDir"])
flags.InDet.Align.inputTracksCollection = kwargs["inputTracksCollection"]
flags.InDet.Align.outputConditionFile = f"{flags.InDet.Align.baseDir}/Solve/{kwargs['outputConditionFile']}"

if kwargs["monitorFile"]:
    flags.InDet.Align.doMonitoring = True
    flags.Output.HISTFileName = f"{flags.InDet.Align.baseDir}/Accumulate/{kwargs['monitorFile']}"

if len(kwargs["inputTFiles"]) == 0:
    flags.InDet.Align.inputTFiles = [f"{flags.InDet.Align.baseDir}/Accumulate/AlignmentTFile.root"] 
    
else:
    flags.InDet.Align.inputTFiles = [os.path.abspath(inputTFile) for inputTFile in kwargs["inputTFiles"]]

flags.Input.Files = [os.path.abspath(inputFile) for inputFile in kwargs["input"]]
flags.Exec.MaxEvents = kwargs["maxEvents"] if not kwargs["solve"] else 1
flags.Exec.OutputLevel = getattr(AthenaCommon.Constants, kwargs["logLevel"])
flags.Exec.FPE = -2
flags.IOVDb.GlobalTag = kwargs["globalTag"]

## If solve step, set output database name
if kwargs["solve"]:
    flags.IOVDb.DBConnection = f"sqlite://;schema={flags.InDet.Align.baseDir}/Solve/{kwargs['outputDBFile']};dbname=CONDBR2"
    
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

# Now flags can be locked:    
flags.lock()

with open(os.devnull, 'w') as f, contextlib.redirect_stdout(f):
    from RecJobTransforms.RecoSteering import RecoSteering
    cfg = RecoSteering(flags)

from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
cfg.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags))

## Accumulate step
if kwargs["accumulate"] and not kwargs["solve"]:
    os.makedirs(f"{flags.InDet.Align.baseDir}/Accumulate", exist_ok = True)
    os.chdir(f"{flags.InDet.Align.baseDir}/Accumulate")
    from InDetAlignConfig.AccumulateConfig import AccumulateCfg
    cfg.merge(AccumulateCfg(flags))

## Solve step
elif kwargs["solve"] and not kwargs["accumulate"]:
    os.makedirs(f"{flags.InDet.Align.baseDir}/Solve", exist_ok = True)
    os.chdir(f"{flags.InDet.Align.baseDir}/Solve")
    from InDetAlignConfig.SolveConfig import SolveCfg
    cfg.merge(SolveCfg(flags))
           
else:
    raise Exception("You can run either the acculumation step or the solve step, but not both or neither at the same time!")
        
## Update condition database (Needs to be done last)
from InDetAlignConfig.IDAlignConditionConfig import UpdateTagsCfg

cfg.merge(UpdateTagsCfg(flags, localDataBase))

##----- Run the setup -----##
                
if kwargs["dryRun"]:
    cfg.printConfig(summariseProps = True)
   
else:
    cfg.run()
