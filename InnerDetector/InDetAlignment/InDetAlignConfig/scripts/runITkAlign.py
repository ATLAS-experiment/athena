#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/scripts/runITkAlign.py
# Authors: David Brunner, Thomas Strebler

import os

from AthenaConfiguration.TestDefaults import (
    defaultConditionsTags,
    defaultGeometryTags,
    defaultTestFiles,
)

def parser():
    from argparse import ArgumentParser

    parser = ArgumentParser(description="Script for ITk alignment")

    # Running mode
    parser.add_argument(
        "-a",
        "--accumulate",
        action="store_true",
        help="Run accumulation step",
    )
    parser.add_argument(
        "-s",
        "--solve",
        action="store_true",
        help="Run solve step",
    )
    parser.add_argument(
        "-d",
        "--dryRun",
        action="store_true",
        help="Configure and print without executing",
    )
    parser.add_argument(
        "-b",
        "--baseDir",
        default="./",
        help="Base directory where output is placed",
    )

    # Input/output
    parser.add_argument(
        "-i",
        "--input",
        default=defaultTestFiles.RDO_RUN4,
        nargs="+",
        help="Input file(s)",
    )
    parser.add_argument(
        "--maxEvents",
        default=-1,
        type=int,
        help="Maximum number of events to process",
    )
    parser.add_argument(
        "-t",
        "--inputTracksCollection",
        default="CombinedITkTracks",
        type=str,
        help="Track collection used by the alignment workflow",
    )
    parser.add_argument(
        "--inputTFiles",
        default="AlignmentTFile.root",
        type=str,
        help="ROOT file produced by MatrixTool in the accumulation step",
    )
    parser.add_argument(
        "--alignmentConstants",
        default=[],
        nargs="+",
        help="Local alignment constants to use",
    )

    # Detector components to align
    parser.add_argument(
        "--alignITk",
        action="store_true",
        help="Align the full ITk",
    )
    parser.add_argument(
        "--alignITkPixel",
        action="store_true",
        help="Align ITk Pixel",
    )
    parser.add_argument(
        "--alignITkStrip",
        action="store_true",
        help="Align ITk Strip",
    )

    # Conditions and geometry tags
    parser.add_argument(
        "--globalTag",
        default=defaultConditionsTags.RUN4_MC,
        help="Global conditions tag",
    )
    parser.add_argument(
        "--atlasVersion",
        default=defaultGeometryTags.RUN4,
        help="ATLAS geometry version",
    )

    parser.add_argument(
        "--isBFieldOff",
        action="store_true",
        help="Run with magnetic field disabled",
    )
    parser.add_argument(
        "--isCosmics",
        action="store_true",
        help="Run with cosmics beam configuration",
    )
    parser.add_argument(
        "--isHeavyIon",
        action="store_true",
        help="Run with heavy-ion configuration",
    )

    # Local geometry
    parser.add_argument(
        "--localgeo",
        action="store_true",
        help="Use local geometry XML files",
    )

    # Local conditions database
    parser.add_argument(
        "--localDB",
        default="",
        help="Use a local SQLite alignment database",
    )
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

# -------------------------------------------------------------------------
# Flags
# -------------------------------------------------------------------------

from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()

from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude

OnlyTrackingPreInclude(flags)

flags.ITk.Align.accumulate = kwargs["accumulate"]
flags.ITk.Align.baseDir = os.path.abspath(kwargs["baseDir"])

flags.ITk.Align.alignITk = (
    kwargs["alignITk"]
    or not (
        kwargs["alignITk"]
        or kwargs["alignITkPixel"]
        or kwargs["alignITkStrip"]
    )
)

flags.ITk.Align.alignITkPixel = (
    kwargs["alignITkPixel"] or flags.ITk.Align.alignITk
)

flags.ITk.Align.alignITkStrip = (
    kwargs["alignITkStrip"] or flags.ITk.Align.alignITk
)

# Folder writing is currently disabled because the corresponding
# ITk output-folder configuration is not complete.
flags.ITk.Align.writeSilicon = False

flags.ITk.Align.inputTFiles = kwargs["inputTFiles"]

flags.Input.Files = kwargs["input"]
flags.Exec.MaxEvents = kwargs["maxEvents"] if not kwargs["solve"] else 1

flags.IOVDb.GlobalTag = kwargs["globalTag"]
flags.GeoModel.AtlasVersion = kwargs["atlasVersion"]
flags.GeoModel.Align.Dynamic = False
if kwargs["threads"] > 0:
   flags.Concurrency.NumThreads = kwargs["threads"]

flags.addFlag(
    "ConstrainedTrackProvider.InputTracksCollection",
    kwargs["inputTracksCollection"],
)

flags.DQ.useTrigger = False
flags.Output.HISTFileName = "IDAlignMon.root"

# -------------------------------------------------------------------------
# Beam and magnetic-field configuration
# -------------------------------------------------------------------------

if not flags.Input.isMC and kwargs["isCosmics"]:
    from AthenaConfiguration.Enums import BeamType

    flags.Beam.NumberOfCollisions = 0
    flags.Beam.Type = BeamType.Cosmics
    flags.Beam.Energy = 0.0
    flags.Beam.BunchSpacing = 50
elif kwargs["isHeavyIon"]:
    flags.Beam.BunchSpacing = 50
    flags.Reco.EnableHI = True
    flags.HeavyIon.doGlobal = True
else:
    flags.Beam.BunchSpacing = 25

field_on = not kwargs["isBFieldOff"]

flags.BField.solenoidOn = field_on
flags.BField.barrelToroidOn = field_on
flags.BField.endcapToroidOn = field_on

# -------------------------------------------------------------------------
# Geometry and local conditions configuration
# -------------------------------------------------------------------------

if kwargs["localgeo"]:
    flags.ITk.Geometry.AllLocal = True

if flags.ITk.Align.alignITkPixel:
    flags.ITk.Geometry.pixelAlignable = True

if flags.ITk.Align.alignITkStrip:
    flags.ITk.Geometry.stripAlignable = True

db_file = ""
db_name = "OFLCOND"
alignment_tag = "InDetSi_MisalignmentMode_random misalignment"

if kwargs["localDB"]:
    flags.ITk.Align.useLocalDatabase = True

    if os.path.isabs(kwargs["localDB"]):
        db_file = os.path.abspath(kwargs["localDB"])
    else:
        db_file = os.path.abspath(
            os.path.join(
                flags.ITk.Align.baseDir,
                kwargs["localDB"],
            )
        )

    if not os.path.exists(db_file):
        raise FileNotFoundError(
            f"Local alignment database does not exist: {db_file}"
        )

    flags.IOVDb.DBConnection = (
        f"sqlite://;schema={db_file};dbname={db_name}"
    )

    flags.ITk.Geometry.alignmentFolder = "/Indet/AlignITk"

flags.lock()

# -------------------------------------------------------------------------
# Main reconstruction configuration
# -------------------------------------------------------------------------

from RecJobTransforms.RecoSteering import RecoSteering

cfg = RecoSteering(flags)

from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg

cfg.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags))

# -------------------------------------------------------------------------
# Alignment workflow
# -------------------------------------------------------------------------

if kwargs["accumulate"]:
    work_dir = os.path.join(
        flags.ITk.Align.baseDir,
        "Accumulate",
    )

    os.makedirs(work_dir, exist_ok=True)
    os.chdir(work_dir)

    if flags.ITk.Align.useLocalDatabase:
        stageLocalDBFiles(db_file)

    from InDetAlignConfig.AccumulateITkConfig import ITkAccumulateCfg

    cfg.merge(ITkAccumulateCfg(flags))

else:
    work_dir = os.path.join(
        flags.ITk.Align.baseDir,
        "Solve",
    )

    os.makedirs(work_dir, exist_ok=True)
    os.chdir(work_dir)

    if flags.ITk.Align.useLocalDatabase:
        stageLocalDBFiles(db_file)

    from InDetAlignConfig.SolveITkConfig import ITkSolveCfg

    cfg.merge(ITkSolveCfg(flags))

# -------------------------------------------------------------------------
# Local alignment conditions input
# -------------------------------------------------------------------------

if flags.ITk.Align.useLocalDatabase:
    from IOVDbSvc.IOVDbSvcConfig import addFolders
 
    cfg.merge(

    addFolders(

        flags,

        flags.ITk.Geometry.alignmentFolder,

        detDb=os.path.basename(db_file),

        db=db_name,

        tag=alignment_tag,

        className="AlignableTransformContainer",

        )

    ) 

##----- Run the setup -----##
                 
if kwargs["dryRun"]:
   cfg.printConfig()    
else:
    cfg.run()
