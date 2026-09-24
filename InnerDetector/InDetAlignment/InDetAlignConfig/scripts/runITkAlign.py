#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runIDAlign.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

import os
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles


def parser():
    from argparse import ArgumentParser

    parser = ArgumentParser(description="Script for ITk alignment")

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

    ## Number of threads
    parser.add_argument("--threads", default = 1, type = int, help='Number of threads')
    

    return parser.parse_args()


def stageLocalDBFiles(db_file):
    """
    Stage the local SQLite database, POOL payload file and POOL catalogue
    in the current accumulation/solve working directory.
    """

    db_file = os.path.abspath(db_file)
    db_dir = os.path.dirname(db_file)
    db_basename = os.path.basename(db_file)
    db_stem = os.path.splitext(db_basename)[0]

    files_to_stage = (
        db_basename,
        f"{db_stem}.pool.root",
        "PoolFileCatalog.xml",
    )

    for filename in files_to_stage:
        source = os.path.join(db_dir, filename)
        destination = os.path.join(os.getcwd(), filename)

        if not os.path.exists(source):
            raise FileNotFoundError(
                f"Required local conditions file does not exist: {source}"
            )

        if os.path.lexists(destination):
            os.remove(destination)

        os.symlink(source, destination)


kwargs = vars(parser())

if kwargs["accumulate"] == kwargs["solve"]:
    raise RuntimeError(
        "Select exactly one running mode: either --accumulate or --solve"
    )

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

if flags.ITk.Align.alignITkPixel:
    flags.ITk.Geometry.pixelAlignable = True
if flags.ITk.Align.alignITkStrip:
    flags.ITk.Geometry.stripAlignable = True

if kwargs["threads"] > 0:
    flags.Concurrency.NumThreads = kwargs["threads"]


# Uncomment for ATLAS-P2-RUN4-04-00-00 / ATLAS-P2-RUN4-05-00-00.
# flags.DQ.useTrigger = False

# Uncomment when running the monitoring configuration to produce IDAlignMon.root.
# flags.Output.HISTFileName = "IDAlignMon.root"


DBFile = ""
DBName = "OFLCOND"
misalignModeMap = {0:'InDetSi_MisalignmentMode_no Misalignment',
                   1: 'InDetSi_MisalignmentMode_misalignment by 6 parameters',
                   2: 'InDetSi_MisalignmentMode_random misalignment',
                   3: 'InDetSi_MisalignmentMode_IBL-stave temperature dependent bowing',
                   7: 'InDetSi_MisalignmentMode_misalignment according to module indices',
                   41: 'InDetSi_MisalignmentMode_ITk endcap beam-pipe z shift',
                   42: 'InDetSi_MisalignmentMode_ITk pixel barrel layer bowing',
                   43: 'InDetSi_MisalignmentMode_ITk barrel radial expansion',
                   11: 'InDetSi_MisalignmentMode_R deltaR (radial expansion)', 12: 'Phi deltaR (ellipse)',13: 'Z deltaR (funnel)',
                   21: 'InDetSi_MisalignmentMode_R deltaPhi (curl)', 22: 'Phi deltaPhi (clamshell) ',23:'Z deltaPhi (twist)',
                   31: 'InDetSi_MisalignmentMode_R deltaZ (telescope)',32:'Phi deltaZ (skew)',33:'Z deltaZ (z-expansion)'}

get_db_name = os.path.basename(kwargs["localDB"])

misalign_mode = int(get_db_name.removeprefix("MisalignmentSet").removesuffix(".db"))
alignment_tag = misalignModeMap.get(int(misalign_mode),'unknown')
if kwargs["localDB"]:
    flags.ITk.Align.useLocalDatabase = True

    if os.path.isabs(kwargs["localDB"]):
        DBFile = os.path.abspath(kwargs["localDB"])
    else:
        DBFile = os.path.abspath(os.path.join(flags.ITk.Align.baseDir,kwargs["localDB"],))

    if not os.path.exists(DBFile):
        raise FileNotFoundError(f"Local alignment database does not exist: {DBFile}")

    flags.IOVDb.DBConnection = (f"sqlite://;schema={DBFile};dbname={DBName}")

    flags.ITk.Geometry.alignmentFolder = "/Indet/AlignITk"


flags.lock()


from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)

if flags.ITk.Align.useLocalDatabase:
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    print("Adding Align Folder "+flags.ITk.Geometry.alignmentFolder+" from local "+DBName+" Database in file "+DBFile)
    cfg.merge(addFolders(flags,flags.ITk.Geometry.alignmentFolder,detDb=os.path.basename(DBFile),db=DBName,tag=alignment_tag,className="AlignableTransformContainer",))

from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
cfg.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags))

## Accumulate step
if kwargs["accumulate"]:

    # First configure the accumulation workflow.
    from InDetAlignConfig.AccumulateITkConfig import ITkAccumulateCfg
    cfg.merge(ITkAccumulateCfg(flags))
    work_dir = os.path.join(flags.ITk.Align.baseDir,"Accumulate",)
    os.makedirs(work_dir,exist_ok=True,)
    os.chdir(work_dir)

    # Make SQLite, POOL payload and POOL catalogue visible from the runtime working directory.
    if flags.ITk.Align.useLocalDatabase:
        stageLocalDBFiles(DBFile)

## Solve step
elif kwargs["solve"] and not kwargs["accumulate"]:

    # First configure the solve workflow.
    from InDetAlignConfig.SolveITkConfig import ITkSolveCfg
    cfg.merge(ITkSolveCfg(flags))

    # Then move to the solve directory.
    work_dir = os.path.join(flags.ITk.Align.baseDir,"Solve",)
    os.makedirs(work_dir,exist_ok=True,)
    os.chdir(work_dir)

    if flags.ITk.Align.useLocalDatabase:
        stageLocalDBFiles(DBFile)

else:
    raise Exception("You can run either the acculumation step or the solve step, but not both or neither at the same time!")

##----- Run the setup -----##
                
if kwargs["dryRun"]:
    cfg.printConfig()
   
else:
    cfg.run()

