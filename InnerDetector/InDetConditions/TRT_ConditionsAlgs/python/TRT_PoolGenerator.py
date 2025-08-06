# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This python script has one main function for the TRT Database upload parameters:
#     - Generaters a DB (mycool.db) and a POOL (pooloutputfile.root) files with a chosen run for Data or MC

# Since we can write those files this module can also read the output setting the flag "--read" for the local DB

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TRTCondWriterCfg(flags, constantsFile, rtTag, t0Tag, **kwargs):

    acc = ComponentAccumulator()
    _db_ = "sqlite://;schema=mycool.db;dbname=OFLP200" if flags.Input.isMC else "sqlite://;schema=mycool.db;dbname=CONDBR2"

    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    acc.merge(IOVDbSvcCfg(flags, dbConnection=_db_ ))

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
    TRTCondStoreText = CompFactory.TRTCondStoreText(name="TRTCondStoreText", CalibInputFile=constantsFile,  **kwargs)
    acc.addCondAlgo(TRTCondStoreText)

    return acc

def TRTCondReaderCfg(flags, sqlite_db, rtTag, t0Tag, outputFile, ReadCOOL, **kwargs):
    acc = ComponentAccumulator()

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    
    if ReadCOOL:
        acc.merge(addFolders( flags, "/TRT/Calib/T0", "TRT_OFL", className="TRTCond::StrawT0MultChanContainer"   ))    
        acc.merge(addFolders( flags, "/TRT/Calib/RT", "TRT_OFL", className="TRTCond::RtRelationMultChanContainer"))

    else:
        # TRT folders from local SQLite
        folderBase = f"sqlite://;schema={sqlite_db};dbname=" + "OFLP200" if flags.Input.isMC else "CONDBR2"
        acc.merge(addFolders( flags, "/TRT/Calib/T0", db=f"{folderBase}", tag=t0Tag, className="TRTCond::StrawT0MultChanContainer"   ))    
        acc.merge(addFolders( flags, "/TRT/Calib/RT", db=f"{folderBase}", tag=rtTag, className="TRTCond::RtRelationMultChanContainer"))
    
    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))

    # TRT CondRead Algorithm
    TRTCondRead = CompFactory.TRTCondRead(name="TRTCondRead", CalibOutputFile=outputFile,  **kwargs)
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
    parser.add_argument('--dbconst', default="dbconst.txt" ,help="Input file constants for writer")
    parser.add_argument('--outputtxt', default="caliboutput.txt" ,help="Output file for the TRTC")
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
    textOutput = "caliboutput.txt"
    if args.condRunNumber > 0:
        flags.Input.RunNumbers = [args.condRunNumber]
        flags.Input.OverrideRunNumber=True
        ReadCOOL = True
        textOutput = f"caliboutput_{args.condRunNumber}.txt"

    flags.Detector.GeometryTRT = True
    flags.Detector.EnableTRT = True
    flags.Output.ESDFileName = "trtcalibout.pool.root"

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
        acc.merge(TRTCondWriterCfg(flags, args.dbconst, args.tagRT, args.tagT0))
    else:
        acc.merge(TRTCondReaderCfg(flags, args.dbname, args.tagRT, args.tagT0, textOutput, ReadCOOL))

    # Run the configuration
    with open("TRT_PoolGenerator.pkl", "wb") as f:
        acc.store(f)
        f.close()

    import sys
    sys.exit(not acc.run().isSuccess())
