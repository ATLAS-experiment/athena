
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
##########################################################################################################
##     Script to read text file and create .db and .pool files with strawstatus constants 		
##########################################################################################################

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def TRTStrawStatusWriteCfg(flags, name="TRTStrawStatusWriteAlg", tagList=[], objectList=[], **kwargs):
    acc = ComponentAccumulator()

    if "StatusInputFile" not in kwargs:
        kwargs.setdefault("StatusInputFile", "straws.txt")

    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    acc.merge(IOVDbSvcCfg(flags))          

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    OutputCond = OutputConditionsAlgCfg(
        flags,
        name="OutputConditionsAlg",
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

    acc.addEventAlgo(CompFactory.TRTStrawStatusWrite(name=name, **kwargs))
    return acc


def TRTStrawStatusReadCfg(flags, name="TRTStrawStatusRead", statusTag="", permTag="", htTag="", ReadCOOL=True, **kwargs):
    acc = ComponentAccumulator()

    if "OutputFile" not in kwargs:
        kwargs.setdefault("OutputFile", "caliboutput.txt")

    if "TRT_StrawStatusSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        kwargs.setdefault("TRT_StrawStatusSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))

    if not ReadCOOL:
        # TRT folders from local SQLite
        from IOVDbSvc.IOVDbSvcConfig import addOverride       
        if kwargs["FolderToPrint"] == "Status":
            acc.merge(addOverride( flags, "/TRT/Cond/Status", tag=statusTag, db=flags.IOVDb.DBConnection))  
        
        if kwargs["FolderToPrint"] == "StatusPermanent":
            acc.merge(addOverride( flags, "/TRT/Cond/StatusPermanent", tag=permTag, db=flags.IOVDb.DBConnection))

        if  kwargs["FolderToPrint"] == "StatusHT":  
            acc.merge(addOverride( flags, "/TRT/Cond/StatusHT", tag=htTag, db=flags.IOVDb.DBConnection))   

    else:
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline 
        acc.merge(addFoldersSplitOnline(flags, "TRT",
                                    onlineFolders  = ["/TRT/Onl/Cond/Status",
                                                      "/TRT/Onl/Cond/StatusPermanent",
                                                      "/TRT/Onl/Cond/StatusHT"], # Argon straw list
                                    offlineFolders = ["/TRT/Cond/Status",
                                                      "/TRT/Cond/StatusPermanent",
                                                      "/TRT/Cond/StatusHT"], # Argon straw list
                                    className = "TRTCond::StrawStatusMultChanContainer"))
 
    # TRT CondRead Algorithm
    TRTStrawStatusRead = CompFactory.TRTStrawStatusRead(name=name,  **kwargs)
    acc.addEventAlgo(TRTStrawStatusRead)

    return acc

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m TRT_ConditionsAlgs.TRT_StatusPoolGenerator',
                                     description="Write or Read TRT conditions. For writing you can provide the three folder, however for reading must be one by one")
    
    parser.add_argument('-r','--read',action='store_true' ,help="By default writes. If set the it reads from local DB")
    parser.add_argument('-m','--isMC',action='store_true' ,help="This is to tell athena if it is MC or Data (OFLP200 or CONDBR2)")
    parser.add_argument('--status'   ,action='store_false' ,help="For straw status tag")
    parser.add_argument('--permanent',action='store_true' ,help="For permanent straws")
    parser.add_argument('--ht'       ,action='store_true',help="For HT straws")
    parser.add_argument('-f','--inputFile', default="straws.txt" ,help="DB folder name for reader")
    parser.add_argument('-c','--condRunNumber', type=int, default=-1, help=" choose the IoV covering this run number")
    parser.add_argument('--dbname', default="mycool.db" ,help="DB folder name for reader")
    parser.add_argument('--outputtxt', default="" ,help="Output file for the TRT")
    
    args = parser.parse_args()


    if not args.status and not args.permanent and not args.ht:
        print("ERROR: you should provide at least one folder --status and/or --permanent and/or --ht")
        exit(1)

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
    flags.Exec.MaxEvents = 1

    # For debug output INFO=3
    flags.Exec.OutputLevel = 3

    flags.lock()
    flags.dump()

    # Main services accumulator
    acc = MainServicesCfg(flags)

    # Add Detector geometry
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc.merge(GeoModelCfg(flags))

    mytagList = []
    myobjectList = []
    folder2read = ""
    
    if args.status:
        folder2read = "Status"
        mytagList.append("TextStatus")
        myobjectList.append("TRTCond::StrawStatusMultChanContainer#/TRT/Cond/Status")

    if not args.read:
        # Add TRT conditions writing
        acc.merge(TRTStrawStatusWriteCfg(flags, StatusInputFile=args.inputFile, tagList=mytagList, objectList=myobjectList))
    else:
        acc.merge(TRTStrawStatusReadCfg(flags, statusTag="TextStatus", permTag="TextPermanent", htTag="TextHT", ReadCOOL=ReadCOOL, OutputFile=textOutput, FolderToPrint=folder2read))    

    
    
    # Run the configuration
    with open("TRT_StatusPoolGenerator.pkl", "wb") as f:
        acc.store(f)
        f.close()

    import sys
    sys.exit(not acc.run().isSuccess())
