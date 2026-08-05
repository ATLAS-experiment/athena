#!/usr/bin/env python
"""
Run PrintHGTDElements

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

import sys
from argparse import ArgumentParser

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import (
    defaultGeometryTags,
    defaultConditionsTags,
)
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

# ------------------------------------------------------------
# Arguments
# ------------------------------------------------------------
parser = ArgumentParser("RunPrintHGTDElements.py")

parser.add_argument(
    "--geometrytag",
    default=defaultGeometryTags.RUN4,
    help="Geometry tag",
)

parser.add_argument(
    "--conditionstag",
    default=defaultConditionsTags.RUN4_MC,
    help="Conditions tag",
)

parser.add_argument(
    "--sqlite",
    default="",
    help="Alignment SQLite file",
)

parser.add_argument(
    "--tag",
    default="",
    help="SQLite COOL tag",
)

parser.add_argument(
    "--geometryfile",
    default="/afs/cern.ch/work/f/fbendebb/HGTD-Alignement/atlas-hgtd-geomodelxml/HGTD_Detector/HGTD.gmx",
    help="Path to the HGTD GMX geometry file",
)

parser.add_argument(
    "--localgeo",
    action="store_true",
    help="Use a local GMX geometry file",
)

args = parser.parse_args()

if args.sqlite and not args.tag:
    parser.error("--tag is required when using --sqlite")

# ------------------------------------------------------------
# Flags
# ------------------------------------------------------------
flags = initConfigFlags()

flags.Input.Files = []
flags.Input.isMC = True

flags.GeoModel.AtlasVersion = args.geometrytag
flags.IOVDb.GlobalTag = args.conditionstag

flags.GeoModel.Align.Dynamic = False

flags.Concurrency.NumThreads = 1

flags.HGTD.Geometry.isLocal = True
flags.HGTD.Geometry.Filename = args.geometryfile

# Enable HGTD
flags.HGTD.Geometry.isAlignable = True
flags.HGTD.Geometry.useGeoModelXml = True

if args.localgeo:
    flags.HGTD.Geometry.isLocal = True
    flags.HGTD.Geometry.Filename = args.geometryfile

setupDetectorFlags(
    flags,
    custom_list=["HGTD"],
    toggle_geometry=True,
)

# Optional SQLite alignment
if args.sqlite:

    flags.IOVDb.DBConnection = (
        f"sqlite://;schema={args.sqlite};dbname=OFLCOND"
    )

    print(f"Using SQLite: {args.sqlite}")
    print(f"Using COOL tag: {args.tag}")

flags.lock()

# ------------------------------------------------------------
# Main services
# ------------------------------------------------------------
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

acc = MainServicesCfg(flags)

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg

acc.merge(IOVDbSvcCfg(flags))

# ------------------------------------------------------------
# Load HGTD alignment from SQLite
# ------------------------------------------------------------
if args.sqlite:

    from IOVDbSvc.IOVDbSvcConfig import addFolders

    acc.merge(
        addFolders(
            flags,
            flags.HGTD.Geometry.alignmentFolder,
            db="OFLCOND",
            detDb=args.sqlite,
            tag=args.tag,
            className="AlignableTransformContainer",
        )
    )

# ------------------------------------------------------------
# HGTD geometry
# ------------------------------------------------------------
from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg

acc.merge(HGTD_ReadoutGeometryCfg(flags))

# ------------------------------------------------------------
# HGTD detector element conditions
# ------------------------------------------------------------
from HGTD_ConditionsAlgorithms.HGTD_ConditionsAlgorithmsConfig import (
    HGTD_DetectorElementCondAlgCfg,
)

acc.merge(HGTD_DetectorElementCondAlgCfg(flags))

# ------------------------------------------------------------
# Print algorithm
# ------------------------------------------------------------
from AthenaConfiguration.ComponentFactory import CompFactory

alg = CompFactory.PrintHGTDElements()

alg.OutputLevel = 2
alg.OutputFile = "HGTDGeometry.dat"

acc.addEventAlgo(alg)

# ------------------------------------------------------------
# Print configuration
# ------------------------------------------------------------
acc.printConfig(
    withDetails=True,
    summariseProps=True,
)

# ------------------------------------------------------------
# Run
# ------------------------------------------------------------
sc = acc.run(1)

sys.exit(not sc.isSuccess())