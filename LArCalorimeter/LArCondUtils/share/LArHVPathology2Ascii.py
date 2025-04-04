#!/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LArHVPathology2AsciiCfg(flags,OutputFile="",InputFile="",folder="/LAR/HVPathologiesOfl/Pathologies",tag=None):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    from LArCabling.LArHVCablingConfig import LArHVCablingCfg
    result.merge(LArHVCablingCfg(flags))


    if tag is not None:
        if not tag.startswith("LAR"):
            if not tag.startswith("-"): tag= "-"+tag
            tag="".join(folder.split("/"))+tag
    
    if (InputFile==""):
        #Reading case
        result.merge(addFolders(flags,folder,"LAR_OFL",tag=tag,
                                className="AthenaAttributeList"))
    

    
        LArHVPathologyDbCondAlg=CompFactory.LArHVPathologyDbCondAlg
        hvpath = LArHVPathologyDbCondAlg(PathologyFolder=folder,
                                        HVMappingKey="LArHVIdMap",
                                        HVPAthologyKey="LArHVPathology")



        result.addCondAlgo(hvpath)


    result.addEventAlgo(CompFactory.LArHVPathologyDbAlg(OutFile=OutputFile,
                                                        InpFile=InputFile,
                                                        Folder=folder,
                                                        WriteCondObjs=(InputFile!=""),
                                                        HVPAthologyKey="LArHVPathology"))
    
    if (InputFile!=""):
        #Writing case:
        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        result.merge(OutputConditionsAlgCfg(flags,
                                        name="AttrListOutputAlg",
                                        outputFile="dummy.pool.root",
                                        ObjectList=["AthenaAttributeList#"+folder+"#"+folder],
                                        IOVTagList=[tag,], 
                                        WriteIOV = True,
                                        Run1=flags.Input.RunNumbers[0],
                                        LB1=0,
                                        Run2=0x7FFFFFFF,
                                        LB2=0xFFFFFFFF
                                        ))
        result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True))

    return result



if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0x1fffffff, type=int, help="run number to query the DB")
    parser.add_argument("-l","--lbnumber",default=1, type=int, help="LB number to query the DB")
    parser.add_argument("-d","--sqlite",default="", help="Sqlite file name (for conditions-creation)")
    parser.add_argument("-o","--output",default="", help="output text file name (for dumping)")
    parser.add_argument("-f","--folder",default="/LAR/HVPathologiesOfl/Pathologies", help="database folder to read or write")
    parser.add_argument("-t","--tag",default=None, help="folder-level tag to read or write")
    parser.add_argument("-i","--input",default="",help="Input Text file (for conditions-creation)")

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()

    flags.Input.isMC = False
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.RunNumbers=[args.runnumber]
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag=defaultConditionsTags.RUN3_DATA
    #flags.Debug.DumpDetStore=True
    if (args.sqlite!=""):
        flags.IOVDb.DBConnection="sqlite://;schema="+args.sqlite+";dbname=CONDBR2"


    if args.loglevel:
        from AthenaCommon import Constants
        if hasattr(Constants,args.loglevel):
            flags.Exec.OutputLevel=getattr(Constants,args.loglevel)
        else:
            raise ValueError("Unknown log-level, allowed values are ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, FATAL")

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
                                 FirstLB=args.lbnumber,
                                 EventsPerRun      = 1,
                                 FirstEvent        = 1,
                                 InitialTimeStamp  = 0,
                                 TimeStampInterval = 1))

    cfg.merge(LArHVPathology2AsciiCfg(flags,
                                      args.output, 
                                      folder=args.folder,
                                      tag=args.tag,
                                      InputFile=args.input
                                     ))
    


    #cfg.setDebugStage("exec")
    sc=cfg.run(1)
    if sc.isSuccess():
        sys.exit(0)
    else:
        sys.exit(1)
