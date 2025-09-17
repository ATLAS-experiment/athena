# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


def LArBadChannelDBAlgCfg(flags,InputFile,dbname="LAR_OFL",folder=None,tag=None,
                          IOVStart=[0,0],IOVEnd=[0x7FFFFFFF,0xFFFFFFFF]):

    logger = logging.getLogger( "LArBadChannelDBAlgCfg" )
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
            
    if tag is None:
        tag="".join(folder.split("/"))+"-RUN2-Bulk-00"

    if not tag.startswith("LAR"):
        if not tag.startswith("-"): tag= "-"+tag
        tag="".join(folder.split("/"))+tag
   
    logger.info("Writing to folder %s, tag %s",folder,tag)
    theLArBadChannelCondAlgo=CompFactory.LArBadChannelCondAlg(ReadKey="",InputFileName=InputFile)
    if flags.LArCalib.isSC:
        theLArBadChannelCondAlgo.CablingKey="LArOnOffIdMapSC"
        theLArBadChannelCondAlgo.isSC=True
        
    result.addCondAlgo(theLArBadChannelCondAlgo)

    #Thats the registration algo
    theLArDBAlg=CompFactory.LArBadChannelDBAlg()
    theLArDBAlg.WritingMode = 0
    theLArDBAlg.DBFolder=folder
    if flags.LArCalib.isSC:
       theLArDBAlg.SuperCell=True
    result.addEventAlgo(theLArDBAlg)

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,"dummy.pool.root",
                                        ObjectList=["CondAttrListCollection#"+folder],
                                        IOVTagList=[tag], 
                                        Run1=IOVStart[0],LB1=IOVStart[1],
                                        Run2=IOVEnd[0],LB2=IOVEnd[1]))

    return result

    
if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("inputfile")
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0, type=int, help="IOV start (runnumber)")
    parser.add_argument("-l","--lbnumber",default=0, type=int, help="IOV start (LB number)")
    parser.add_argument("--runnumber2",default=0x7FFFFFFF, type=int, help="IOV end (runnumber)")
    parser.add_argument("--lbnumber2",default=0xFFFFFFFF, type=int, help="IOV end (LB number)")
    parser.add_argument("-o","--output",default="BadChannels.db", help="sqlite output file name")
    parser.add_argument("-f","--folder",default=None, help="database folder to create")
    parser.add_argument("-t","--tag",default=None, help="folder-level tag (or tag-suffix) to create")
    parser.add_argument("--SC", action='store_true', help="Work on SuperCells")
    parser.add_argument("--MC", action='store_true', help="Work on OFLP200")


    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags)

    flags.Input.Files=[]
    flags.Input.isMC = args.MC
    from Campaigns.Utils import Campaign
    flags.Input.MCCampaign = Campaign.Unknown
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.RunNumbers=[args.runnumber if args.runnumber>0 else 300000]
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.IOVDb.GlobalTag=defaultConditionsTags.RUN3_MC if flags.Input.isMC else defaultConditionsTags.RUN3_DATA22
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.LArCalib.isSC=args.SC
    flags.IOVDb.DBConnection=f"sqlite://;schema={args.output};dbname={flags.IOVDb.DatabaseInstance}"

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

    cfg.merge(LArBadChannelDBAlgCfg(flags, 
                                    args.inputfile,
                                    folder=args.folder,
                                    tag=args.tag,
                                    IOVStart=[args.runnumber,args.lbnumber],
                                    IOVEnd=[args.runnumber2,args.lbnumber2]
                                    ))
    


    sc=cfg.run(1)
    if sc.isSuccess():
        sys.exit(0)
    else:
        sys.exit(1)
