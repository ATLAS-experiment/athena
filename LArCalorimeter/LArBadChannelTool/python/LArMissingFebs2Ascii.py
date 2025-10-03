# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders


def LArBadFebs2AsciiCfg(flags,OutputFile,dbname="LAR_OFL",folder=None,tag=None):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    
    if folder is None:
        if dbname == "LAR_ONL":
            folder="/LAR/BadChannels/KnownBADFEBs"
        else: 
            folder="/LAR/BadChannelsOfl/KnownBADFEBs"

    if tag is not None:
        if not tag.startswith("LAR"):
            if not tag.startswith("-"): tag= "-"+tag
            tag="".join(folder.split("/"))+tag
   
        print("Tag=",tag)

    result.merge(addFolders(flags,folder,dbname,tag=tag,
                            className="AthenaAttributeList"))

    result.addCondAlgo(CompFactory.LArBadFebCondAlg("LArKnownBadFebAlg",ReadKey=folder,WriteKey="LArBadFeb"))

    result.addEventAlgo(CompFactory.LArBadFeb2Ascii(FileName=OutputFile))
                        
    return result



if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0x7fffffff, type=int, help="run number to query the DB")
    parser.add_argument("-l","--lbnumber",default=1, type=int, help="LB number to query the DB")
    parser.add_argument("-d","--database",default="LAR_OFL", help="Database name or sqlite file name")
    parser.add_argument("-o","--output",default="bf_output.txt", help="output file name")
    parser.add_argument("-f","--folder",default=None, help="database folder to read")
    parser.add_argument("-t","--tag",default=None, help="folder-level tag to read")

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags)
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.RunNumbers=[args.runnumber]
    flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2022-06"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    
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

    cfg.merge(LArBadFebs2AsciiCfg(flags,args.output, 
                                  dbname=args.database,
                                  folder=args.folder,
                                  tag=args.tag))
    


    sc=cfg.run(1)
    if sc.isSuccess():
        sys.exit(0)
    else:
        sys.exit(1)
