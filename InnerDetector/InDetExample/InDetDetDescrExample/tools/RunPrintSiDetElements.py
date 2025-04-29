#!/usr/bin/env python
"""Run PrintSiDetectorElements

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
import sys
from argparse import ArgumentParser

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultGeometryTags,defaultConditionsTags

# Argument parsing
parser = ArgumentParser("PrintSiDetectorElements.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--localgeo", default=False, action="store_true",
                    help="Use local geometry XML files")
parser.add_argument("--geometrytag", default=defaultGeometryTags.RUN4, type=str,
                    help="The geometry tag to use")
parser.add_argument("--conditionstag", default=defaultConditionsTags.RUN4_MC, type=str,
                    help="The geometry tag to use")
parser.add_argument("--sqlitefile",default="", type=str,
                    help="Specify an SQLite geometry input file to use the Run4 SQL-based geometry workflow, rather than standard geometry")
parser.add_argument("--misalignmode",default=None, type=int,
                    help="misalignment Mode")
args = parser.parse_args()


# Some info about the job
print("----PrintSiDetectorElements----")
print()
if args.localgeo:
    print("Using local Geometry XML files")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()

# Configure
flags = initConfigFlags()
flags.Concurrency.NumThreads = 1
if flags.Concurrency.NumThreads > 0:
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True

flags.GeoModel.Align.Dynamic = False
flags.GeoModel.AtlasVersion = args.geometrytag
flags.Input.isMC = True
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
flags.Input.Files = []

flags.GeoModel.Align.Dynamic = False

MisalignMode = args.misalignmode
if args.misalignmode is not None:
    tag="InDetSi_MisalignmentMode_random misalignment"
    BFile=""
    DBFile="MisalignmentSet"+str(MisalignMode)+".db"
    flags.ITk.Geometry.alignmentFolder = "/Indet/AlignITk"
    flags.ITk.Geometry.pixelAlignable = True
    flags.ITk.Geometry.stripAlignable = True
    if(MisalignMode==0):
        tag="InDetSi_MisalignmentMode_no Misalignment"
    elif(MisalignMode==1):
        tag="InDetSi_MisalignmentMode_misalignment by 6 parameters"
    elif(MisalignMode==2):
        tag="InDetSi_MisalignmentMode_random misalignment"
    elif(MisalignMode==3):
        tag="InDetSi_MisalignmentMode_IBL-stave temperature dependent bowing"
    elif(MisalignMode==11):
        tag="InDetSi_MisalignmentMode_R deltaR (radial expansion)"
    elif(MisalignMode==21):
        tag="InDetSi_MisalignmentMode_R deltaPhi (curl)"
    elif(MisalignMode==31):
        tag="InDetSi_MisalignmentMode_R deltaZ (telescope)"
    elif(MisalignMode==99):
        tag="InDetSi_MisalignmentMode_99"
        DBFile="MisalignmentSet99.db"
    DBName="OFLCOND"
    flags.IOVDb.DBConnection ="sqlite://;schema="+DBFile+";dbname="+DBName

if args.localgeo:
    flags.ITk.Geometry.AllLocal = True

elif args.sqlitefile:
    print("Using SQLite input")
    flags.GeoModel.SQLiteDB = True
    from AtlasGeoModel import CommonGeoDB
    CommonGeoDB.SetupLocalSqliteGeometryDb(args.sqlitefile,args.geometrytag)


from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(flags, args.detectors, toggle_geometry=True)



flags.lock()

# Construct our accumulator to run
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaConfiguration.ComponentFactory import CompFactory
# Pixel
if flags.Detector.EnablePixel:
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc.merge(PixelReadoutGeometryCfg(flags))
    ReadPixelDetElements = CompFactory.ReadSiDetectorElements('ReadPixelDetElements')
    ReadPixelDetElements.ManagerName = "Pixel"
    ReadPixelDetElements.DetEleCollKey = "PixelDetectorElementCollection"
    ReadPixelDetElements.UseConditionsTools = False
    acc.addEventAlgo(ReadPixelDetElements)

    PrintPixelDetElements = CompFactory.PrintSiElements('PrintPixelDetElements')
    PrintPixelDetElements.OutputLevel = 5
    PrintPixelDetElements.DetectorManagerNames = ["Pixel"]
    PrintPixelDetElements.OutputFile = "PixelGeometry.dat"
    acc.addEventAlgo(PrintPixelDetElements)

# SCT
if flags.Detector.EnableSCT:
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

    ReadSCTDetElements = CompFactory.ReadSiDetectorElements('ReadSCTDetElements')
    ReadSCTDetElements.ManagerName = "SCT"
    ReadSCTDetElements.DetEleCollKey = "SCT_DetectorElementCollection"
    ReadSCTDetElements.UseConditionsTools = False
    acc.addEventAlgo(ReadSCTDetElements)

    PrintSCTDetElements = CompFactory.PrintSiElements('PrintSCTDetElements')
    PrintSCTDetElements.OutputLevel = 5
    PrintSCTDetElements.DetectorManagerNames = ["SCT"]
    PrintSCTDetElements.ModulesOnly = False
    PrintSCTDetElements.OutputFile = "SCT_Geometry.dat"
    acc.addEventAlgo(PrintSCTDetElements)

if args.misalignmode is not None:
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    print("Adding Align Folder")
    acc.merge(addFolders(flags,flags.ITk.Geometry.alignmentFolder,db=DBName,detDb=DBFile,tag=tag))

# ITk Pixel
if flags.Detector.EnableITkPixel:
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    ReadPixelDetElements = CompFactory.ReadSiDetectorElements('ReadITkPixelDetElements')
    ReadPixelDetElements.ManagerName = "ITkPixel"
    ReadPixelDetElements.DetEleCollKey = "ITkPixelDetectorElementCollection"
    ReadPixelDetElements.UseConditionsTools = False
    acc.addEventAlgo(ReadPixelDetElements)

    PrintPixelDetElements = CompFactory.PrintSiElements('PrintITkPixelDetElements')
    PrintPixelDetElements.OutputLevel = 5
    PrintPixelDetElements.DetectorManagerNames = ["ITkPixel"]
    PrintPixelDetElements.OutputFile = "PixelGeometry.dat"
    if args.misalignmode is not None:
        PrintPixelDetElements.AlignedPosition = True
        print ("Pixel aligned position written into dat")
    acc.addEventAlgo(PrintPixelDetElements)


# ITk Strip
if flags.Detector.EnableITkStrip:
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    ReadStripDetElements = CompFactory.ReadSiDetectorElements('ReadStripDetElements')
    ReadStripDetElements.ManagerName = "ITkStrip"
    ReadStripDetElements.DetEleCollKey = "ITkStripDetectorElementCollection"
    ReadStripDetElements.UseConditionsTools = False
    acc.addEventAlgo(ReadStripDetElements)

    PrintStripDetElements = CompFactory.PrintSiElements('PrintStripDetElements')
    PrintStripDetElements.OutputLevel = 5
    PrintStripDetElements.DetectorManagerNames = ["ITkStrip"]
    PrintStripDetElements.ModulesOnly = False
    PrintStripDetElements.OutputFile = "StripGeometry.dat"
    if args.misalignmode is not None:
        PrintStripDetElements.AlignedPosition = True
        print ("Strip aligned position written into dat")
    acc.addEventAlgo(PrintStripDetElements)

# Execute and finish
sc = acc.run(maxEvents=1)

# Success should be 0
sys.exit(not sc.isSuccess())
