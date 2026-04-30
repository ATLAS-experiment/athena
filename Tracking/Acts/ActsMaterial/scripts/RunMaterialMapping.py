#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Run material mapping
"""
from AthenaCommon.Logging import log
from argparse import ArgumentParser

def dict_from_key_value(arg : str) -> dict:
    """Convert a 'key1=val1,key2=val2' string into a dictionary with typed values."""
    import argparse
    dict = {}
    items = arg.split(',')
    for item in items:
        if '=' not in item:
            raise argparse.ArgumentTypeError(
                f"Invalid key=value pair: '{item}'. Use format 'key=value'."
            )
        key, value = item.split('=', 1)

        
        if type(key) != str:
             raise argparse.ArgumentTypeError(f"Key should be a string '{key}'")     
             
        dict[key.strip()] = int(value)
    return dict

def SetupArgParser():
    from argparse import ArgumentParser
   
    import sys
    # Argument parsing
    parser = ArgumentParser("RunMaterialMapping.py")
    parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                        help="Specify the list of detectors")
    parser.add_argument("--localgeo", default=False, action="store_true",
                        help="Use local geometry Xml files")
    parser.add_argument("--geoModelSqLiteFile", default = "", help="Read geometry from sqlite file")
    parser.add_argument("-V", "--verboseAccumulators", default=False,
                        action="store_true",
                        help="Print full details of the AlgSequence")
    parser.add_argument("--storeTracks", default = False, action="store_true", 
                        help="Store the mapped and unmapped geantino tracks" )
    parser.add_argument("--storeSurface", default = False, action="store_true", 
                        help="Store the surface info on the mapped/unmapped tracks" )
    parser.add_argument("-S", "--verboseStoreGate", default=False,
                        action="store_true",
                        help="Dump the StoreGate(s) each event iteration")
    parser.add_argument("--maxEvents",default=-1, type=int,
                        help="The number of events to run. 0 skips execution")
    parser.add_argument("--skipEvents",default=0, type=int,
                        help="The number of events to skip")
    parser.add_argument("--batchSize", default=1000, type = int, 
                        help="Number of Material events per host event")
    parser.add_argument("--treeName", help="Name of the input tree in the file",
                        default="material-tracks", type=str)
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    parser.add_argument("--geometrytag",default=defaultGeometryTags.RUN4, type=str,
                        help="The geometry tag to use")
    parser.add_argument("--inputFiles", type=str, nargs="+",
                        default=[],
                        help="Input files to be used for the mapping procedure. They must contain the material track information, which was previously produced with the 'RunGeantinoMaterialTrackProduction.py'")
    
    parser.add_argument("--materialBins", default=[], help = "Binning on the surfaces for the material map as key value pairs in a single str (e.g --materialBins 'nPhiBIns=12 nZBins=10')",
                         type=dict_from_key_value)
    return parser

def assembleFiles(fileArgs):
    from os import path, listdir
    outList = []
    for fileArg in fileArgs:
        if path.isdir(fileArg):
            outList += [ f"{fileArg}/{y}" for y in listdir(fileArg) ]
        else:
            if fileArg[fileArg.rfind(".")+1 :] not in ["txt", "conf"]:
                 outList+=[fileArg]
            else:
                with open(fileArg) as inStream:
                   outList+=[ line.strip() for line in inStream if line[0]!='#'] 

    return outList


if __name__ == "__main__":

    args = SetupArgParser().parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # Some info about the job
    print("----RunMaterialMapping for ITk geometry----")
    print()
    print("Using Geometry Tag: "+args.geometrytag)
    if args.localgeo:
        print("...overridden by local Geometry Xml files")
    print("Input material track file:\n{files}".format(files = "  -- \n".join(args.inputFiles)))
    if not args.detectors:
        print("Running complete detector")
    else:
        print("Running with: {}".format(", ".join(args.detectors)))
    print()

    if args.storeSurface and not args.storeTracks:
        print("Wrong configuration- I cannot store surface info without storing the geantino tracks!")
        exit()

    # Configure
    flags = initConfigFlags()
    if args.localgeo:
        flags.ITk.Geometry.AllLocal = True
    from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults, configureDefaultTagsCfg
    flags.Input.Files = MuonPhaseIITestDefaults.EVGEN_PG
    flags.Input.isMC=True
    flags.GeoModel.AtlasVersion = args.geometrytag
    flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
    flags.GeoModel.Align.Dynamic = False

    # This should run serially
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.FPE= 500
    flags.Exec.EventPrintoutInterval = 500
    if args.maxEvents > 0:
        procHostEvents = int(args.maxEvents / args.batchSize) + 2
        flags.Exec.MaxEvents = procHostEvents
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors, setupDetectorFlags
    from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors
    
    ### Don't setup the active muon material in the Acts tracking goemerty
    ### but the passive material representing the coils etc.
    flags.Muon.trackGeometryActiveMaterial= False
    flags.Muon.trackGeometryPassiveMaterial= True

    if args.geoModelSqLiteFile:
         flags.GeoModel.SQLiteDB = True
         flags.GeoModel.SQLiteDBFullPath = args.geoModelSqLiteFile
         # hack to set Run4 for running on muon dead material geometry
         from AthenaConfiguration.Enums import LHCPeriod
         flags.GeoModel.Run = LHCPeriod.Run4
         configureDefaultTagsCfg(flags)
    else:
        defaultDetectors = ['ITkPixel', 'ITkStrip']
        detectors = args.detectors if 'detectors' in args and args.detectors else defaultDetectors
        detectors.append('Bpipe')  # always run with beam pipe
        setupDetectorFlags(flags, detectors, toggle_geometry=True)

    flags.Acts.TrackingGeometry.UseBlueprint = True

    #flags.Exec.SkipEvents = args.skipEvents
    
    log.debug('Lock config flags now.')
    flags.lock()

    # Construct our accumulator to run
    acc = MainServicesCfg(flags)

    acc.getService("MessageSvc").verboseLimit = 10000000
    acc.getService("MessageSvc").debugLimit = 10000000
    acc.getService("MessageSvc").errorLimit = 10000000


    ### setup dumping of additional information
    if args.verboseAccumulators: acc.printConfig(withDetails=True)
    if args.verboseStoreGate: acc.getService("StoreGateSvc").Dump = True

    log.debug('Dumping of ConfigFlags now.')
    flags.dump()

    from ActsConfig.ActsMaterialConfig import MaterialTrackReaderCfg, MaterialMappingCfg
    import sys

    acc.merge(MaterialTrackReaderCfg(flags, 
                                     maxEvents =  args.maxEvents if args.maxEvents > 0 else sys.maxsize,
                                     skipEvents = args.skipEvents,
                                     batchSize = args.batchSize,
                                     FileNames=assembleFiles(args.inputFiles),
                                     TreeName=args.treeName))

    acc.merge(MaterialMappingCfg(flags, 
                                 StoreTracks=args.storeTracks,
                                 StoreSurfInfo=args.storeSurface))

    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    if flags.Detector.GeometryMuon:
        builder = acc.getService("ActsTrackingGeometrySvc").BlueprintNodeBuilders["MuonBlueprintNodeBuilder"]
        if args.materialBins:
            bins_dict = args.materialBins
            for key,value in bins_dict.items():
                setattr(builder, key, value)

    
    executeTest(acc)    


