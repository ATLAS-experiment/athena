# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LArBadChannel2AsciiCfg(flags,OutputFile,dbname="LAR_OFL",folder=None,tag=None,summaryfile=""):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)

    if flags.LArCalib.isSC:
        #Setup SuperCell cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
        result.merge(LArOnOffIdMappingSCCfg(flags))
    else:
        #Setup regular cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
        result.merge(LArOnOffIdMappingCfg(flags))

    
    if folder is None:
        if dbname in ("LAR","LAR_ONL") or flags.Input.isMC:
            folder="/LAR/BadChannels/BadChannels"
        else: 
            folder="/LAR/BadChannelsOfl/BadChannels"

        if flags.LArCalib.isSC:
            folder+="SC"

    if tag is not None:
        if not tag.startswith("LAR"):
            if not tag.startswith("-"): tag= "-"+tag
            tag="".join(folder.split("/"))+tag
   
        print("Tag=",tag)

    result.merge(addFolders(flags,folder,dbname,tag=tag,
                           className="CondAttrListCollection"))
    theLArBadChannelCondAlgo=CompFactory.LArBadChannelCondAlg(ReadKey=folder)
    if flags.LArCalib.isSC:
        theLArBadChannelCondAlgo.CablingKey="LArOnOffIdMapSC"
        theLArBadChannelCondAlgo.isSC=True

    result.addCondAlgo(theLArBadChannelCondAlgo)

    if summaryfile!="":
        if (not flags.LArCalib.isSC):
            from LArBadChannelTool.LArBadFebsConfig import LArKnownBadFebCfg
            result.merge(LArKnownBadFebCfg(flags))
    
    theLArBadChannels2Ascii=CompFactory.LArBadChannel2Ascii(SkipDisconnected=True)
    theLArBadChannels2Ascii.FileName=OutputFile
    theLArBadChannels2Ascii.WithMissing=False if (summaryfile=="" and not flags.LArCalib.isSC) else True
    theLArBadChannels2Ascii.ExecutiveSummaryFile=summaryfile
    theLArBadChannels2Ascii.BFKey="LArKnownBadFEBs"
    if (flags.LArCalib.isSC):
        theLArBadChannels2Ascii.LArOnOffIdMapKey="LArOnOffIdMapSC"
        theLArBadChannels2Ascii.SuperCell=True
    result.addEventAlgo(theLArBadChannels2Ascii)

    return result



if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0x7fffffff, type=int, help="run number to query the DB")
    parser.add_argument("-l","--lbnumber",default=1, type=int, help="LB number to query the DB")
    parser.add_argument("-d","--database",default="LAR_OFL", help="Database name or sqlite file name")
    parser.add_argument("-o","--output",default="bc_output.txt", help="output file name")
    parser.add_argument("-f","--folder",default=None, help="database folder to read")
    parser.add_argument("-t","--tag",default=None, help="folder-level tag to read")
    parser.add_argument("-s","--summary",default="", help="Executive summary file")
    parser.add_argument("--SC", action='store_true', help="Work on SuperCells")
    parser.add_argument("--MC", action='store_true', default=False, help="Work on MC DB")

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags)
    flags.Input.Files = []
    flags.Input.isMC = args.MC
    flags.IOVDb.DatabaseInstance="OFLP200" if args.MC else "CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.RunNumbers=[args.runnumber]
    flags.IOVDb.GlobalTag="OFLCOND-MC21-SDR-RUN3-11" if args.MC else "CONDBR2-ES1PA-2023-02"
    if args.MC:
       from Campaigns.Utils import Campaign
       flags.Input.MCCampaign = Campaign.MC21a

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.LArCalib.isSC=args.SC
    
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

    cfg.merge(LArBadChannel2AsciiCfg(flags,args.output, 
                                     dbname=args.database,
                                     folder=args.folder,
                                     tag=args.tag,
                                     summaryfile=args.summary))
    


    sc=cfg.run(1)
    if sc.isSuccess():
        sys.exit(0)
    else:
        sys.exit(1)
