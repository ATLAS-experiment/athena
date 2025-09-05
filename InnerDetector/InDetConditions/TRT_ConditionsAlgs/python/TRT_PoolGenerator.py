# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This python script has one main function for the TRT Database upload parameters:
#     - Generaters a DB (mycool.db) and a POOL (pooloutputfile.root) files with a chosen run for Data or MC

# Since we can write those files this module can also read the output setting the flag "--read" for the local DB

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TRTCondWriterCfg(flags, name="TRTCondStoreText", rtTag="Textrt", t0Tag="Textt0", **kwargs):

    acc = ComponentAccumulator()

    if "CalibInputFile" not in kwargs:
        kwargs.setdefault('CalibInputFile', "dbconst.txt")
    
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    acc.merge(IOVDbSvcCfg(flags))

    # Define OutputConditionsAlg
    objectList = [
        "TRTCond::RtRelationMultChanContainer#/TRT/Calib/RT",
        "TRTCond::StrawT0MultChanContainer#/TRT/Calib/T0"
    ]
    tagList = [rtTag, t0Tag]

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    OutputCond = OutputConditionsAlgCfg(
        flags,
        name="TRT_OutputConditionsAlg",
        outputFile="pooloutputfile.root",
        ObjectList=objectList,
        IOVTagList=tagList,
        WriteIOV=True,
        Run1=0,
        LB1=0,
        Run2=2147483647,
        LB2=4294967295
    )
    acc.merge(OutputCond)

    # Add pool output stream tool
    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))

    # TRT Conditions text reader
    TRTCondStoreText = CompFactory.TRTCondStoreText(name=name, **kwargs)
    acc.addCondAlgo(TRTCondStoreText)

    return acc

def TRTCondReaderCfg(flags, name="TRTCondRead", rtTag="Textrt", t0Tag="Textt0", ReadCOOL=True, **kwargs):
    acc = ComponentAccumulator()

    if "CalibOutputFile" not in kwargs:
        kwargs.setdefault('CalibOutputFile', "caliboutput.txt")

    from IOVDbSvc.IOVDbSvcConfig import addOverride
    
    #Folder for COOL db are added in the TRTCalDbTool tool, in case of local DB overwrite with the local tag!
    if not ReadCOOL:
        # TRT folders from local SQLite
        acc.merge(addOverride( flags, "/TRT/Calib/T0", tag=t0Tag, db=flags.IOVDb.DBConnection))    
        acc.merge(addOverride( flags, "/TRT/Calib/RT", tag=rtTag, db=flags.IOVDb.DBConnection))

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))
    
    # TRT CondRead Algorithm
    TRTCondRead = CompFactory.TRTCondRead(name=name, **kwargs)
    acc.addEventAlgo(TRTCondRead)

    return acc

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m TRT_ConditionsAlgs.TRT_PoolGenerator --read --condRunNumber 450000',
                                     description="Write or Read TRT conditions")
    
    parser.add_argument('-r','--read',action='store_true' ,help="By default writes. If set the it reads from local DB")
    parser.add_argument('-m','--isMC',action='store_true' ,help="This is to tell athena if it is MC or Data (OFLP200 or CONDBR2)")
    parser.add_argument('--tagRT', default="Textrt" ,help="Tag for RT folder")
    parser.add_argument('--tagT0', default="Textt0" ,help="Tag for T0 folder")
    parser.add_argument('--dbname', default="mycool.db" ,help="DB folder name for reader")
    parser.add_argument('-f','--inputFile', default="dbconst.txt" ,help="Input file constants for writer")
    parser.add_argument('--outputtxt', default="" ,help="Output file for the TRT")
    parser.add_argument('--condRunNumber', type=int, default=-1, help=" choose the IoV covering this run number")
    args = parser.parse_args()

    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles, defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    if args.isMC:
        flags.Input.Files = defaultTestFiles.RDO_RUN3 
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    else:
        flags.Input.Files = defaultTestFiles.RAW_RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA

    ReadCOOL = False
    textOutput = args.outputtxt if args.outputtxt else "caliboutput.txt"
    if args.condRunNumber > 0:
        flags.Input.RunNumbers = [args.condRunNumber]
        flags.Input.OverrideRunNumber=True
        ReadCOOL = True
        textOutput = args.outputtxt if args.outputtxt else f"caliboutput_{args.condRunNumber}.txt"
    else:
        flags.IOVDb.DBConnection = f"sqlite://;schema={args.dbname};dbname=" + ("OFLP200" if args.isMC else "CONDBR2")


    flags.Detector.GeometryTRT = True
    flags.Detector.EnableTRT = True
    flags.Output.ESDFileName = "trtcalibout.pool.root"

    # For debug output INFO=3
    flags.Exec.OutputLevel = 3

    flags.Exec.MaxEvents = 1
    flags.lock()
    flags.dump()

    # Main services accumulator
    acc = MainServicesCfg(flags)

    # Add Detector geometry
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc.merge(GeoModelCfg(flags))

    if not args.read:
        # Add TRT conditions writing
        acc.merge(TRTCondWriterCfg(flags, rtTag=args.tagRT, t0Tag=args.tagT0, CalibInputFile=args.inputFile))
    else:
        acc.merge(TRTCondReaderCfg(flags, rtTag=args.tagRT, t0Tag=args.tagT0, ReadCOOL=ReadCOOL, CalibOutputFile=textOutput))

    # Run the configuration
    with open("TRT_PoolGenerator.pkl", "wb") as f:
        acc.store(f)
        f.close()

    import sys
    sys.exit(not acc.run().isSuccess())
