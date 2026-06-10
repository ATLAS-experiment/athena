#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Configure and initialize ITkPixelCsvWaferIdAlg.

This script is a lightweight AthenaConfiguration entry point for the CSV-based
wafer identifier algorithm. It currently validates that the algorithm can be
configured and executed, and prints the lookup request that would be passed to
its waferId() implementation.
"""

from argparse import ArgumentParser

from AthenaCommon.Constants import INFO
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Logging import log
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg

from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelCsvWaferIdAlgCfg

parser = ArgumentParser("RunITkPixelCsvWaferIdAlg.py")
parser.add_argument(
    "--csv-file",
    default="/eos/atlas/atlascerngroupdisk/det-itk/general/pixels/identifiers/AT2-IP-ES-0016_v1.41_INCOMPLETE-ModuleA_slim.csv",
    help="CSV file to load. The default is resolved through DATAPATH.",
)
parser.add_argument(
    "--output-file",
    default="ITkPixelWaferIds.txt",
    help="Output text file for one 32-bit waferID identifier per line.",
)

parser.add_argument(
    "--verbose",
    action="store_true",
    help="Print the full Athena configuration and properties.",
)
args = parser.parse_args()

log.setLevel(INFO)

flags = initConfigFlags()
flags.Input.isMC = True
flags.Input.Files = []  # No input files needed for this test

#flags.Exec.OutputLevel=DEBUG

from AthenaConfiguration.TestDefaults import defaultGeometryTags
flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
flags.GeoModel.Align.Dynamic = False

from AthenaConfiguration.TestDefaults import defaultConditionsTags
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

flags.lock()

cfg = MainServicesCfg(flags)
cfg.merge(ITkPixelReadoutGeometryCfg(flags))
cfg.merge(ITkPixelCsvWaferIdAlgCfg(flags,
                                       CsvFile=args.csv_file,
                                       OutputFile=args.output_file))

if args.verbose:
    cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

# Run the application to initialize and execute
cfg.run(1)

print("Configured ITkPixelCsvWaferIdAlg")
print(f"  CSV file: {args.csv_file}")
print(f"  Output file: {args.output_file}")
print("  Note: the algorithm execution writes one 32-bit waferID+feID per line to the output file.")
