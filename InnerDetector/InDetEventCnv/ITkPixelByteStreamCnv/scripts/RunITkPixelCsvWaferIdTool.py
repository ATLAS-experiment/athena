#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Configure and initialize ITkPixelCsvWaferIdAlg.

This script is a lightweight AthenaConfiguration entry point for the CSV-based
wafer identifier algorithm. It currently validates that the algorithm can be
configured and executed, and prints the lookup request that would be passed to
its waferId() implementation.
"""

from argparse import ArgumentParser, BooleanOptionalAction

from AthenaCommon.Constants import INFO
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Logging import log
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg

from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelCsvWaferIdAlgCfg

import csv, json, sys, os
import chai_addDID

parser = ArgumentParser("RunITkPixelCsvWaferIdAlg.py")
parser.add_argument(
    "--csv-file",
    default="/eos/atlas/atlascerngroupdisk/det-itk/general/pixels/identifiers/AT2-IP-ES-0016_v1.41_INCOMPLETE-ModuleA_slim.csv",
    help="CSV file to load. The default is resolved through DATAPATH."
)
parser.add_argument(
    "--output-file",
    default="ITkPixelWaferIds.txt",
    help="Output csv file for one 32-bit waferID identifier per line."
)
parser.add_argument(
    "--crest-upload",
    action=BooleanOptionalAction,
    default="False",
    help="Upload to CREST database."
)
parser.add_argument(
    "--crest-tag",
    default="ITkPixModIDMap-RUN4-00-00-TEST",
    help="TAG used for CREST upload."
)
parser.add_argument(
    "--crest-db",
    default="crest:https://atlas-crest-dev.cern.ch/api-v6.4",
    help="CREST server address, of the type: crest:https://atlas-crest-dev.cern.ch/api-v6.4"
)
parser.add_argument(
    "--crest-since",
    default="0",
    help="start time of IOV"
)
parser.add_argument(
    "--crest-until",
    default="0xFFFFFFFFFFFFFFFF",
    help="end time of IOV"
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
print(f"  Input CSV file: {args.csv_file}")
print(f"  Output file: {args.output_file}")
print("  Note: the algorithm execution writes one front end per line to the output file.")
print("  32-bit waferID+feID, 32-bit waferID+feID, FELIX Card Name, Uplink Pin, DMA buffer, SourceID")

print("  Converting csv to json....")

print("JSON retaining:  32-bit waferID+feID, 32-bit waferID+feID, SourceID")
if os.path.isfile(args.output_file+".json"):
    print(f"File {args.output_file}.json exists, will overwrite it")

with open(args.output_file+".json", "w") as f, open(args.output_file, newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    fieldnames = reader.fieldnames or []
    desired_names = ["DetectorResourceID", "TrueDetectorResourceID", "SourceID"]
    selected_names = [name for name in desired_names if name in fieldnames]
    if not selected_names:
        selected_indices = [0, 1, 5]
        selected_names = [fieldnames[i] for i in selected_indices if i < len(fieldnames)]
    rows = [{name: row[name] for name in selected_names} for row in reader]
    f.write(json.dumps(rows))


print("Wrote output !")

# TODO make if statement, print argument...
if(args.crest_upload == True):
    print("Now uploading to CREST Conditions DataBase")

    chai_addDID.main([ 
        "--tag", args.crest_tag,
        "--db", args.crest_db,
        "--dataA", args.output_file+".json",
        "--since", args.crest_since,
        "--until", args.crest_until
        ])


print(" CREST upload  Done !!")