#!/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def CaloNoise2NtupleCfg(flags,outputFile):

    cfg=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    cfg.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))
    cfg.merge(CaloNoiseCondAlgCfg(flags,"pileupNoise"))

    cfg.addEventAlgo(CompFactory.CaloNoise2Ntuple())
    cfg.addService(CompFactory.THistSvc(Output  = ["file1 DATAFILE='"+outputFile+"' OPT='RECREATE'"]))

    return cfg
    
if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0x7fffffff, type=int, help="run number to query the DB")
    parser.add_argument("-l","--lbnumber",default=1, type=int, help="LB number to query the DB")
    parser.add_argument("-d","--database",default=None, help="Database name or sqlite file name")
    parser.add_argument("-o","--output",default="caloNoise.root", help="output file name")
    parser.add_argument("-t","--tag",default=None, help="folder-level tag to read")
    parser.add_argument("-m","--mc", action='store_true', help="data or MC?")
    parser.add_argument("-p", "--pileup", default=-1., type=float, help="if positive, calculate the noise for a fixed <mu> = specified value")
    parser.add_argument("--online", action='store_true', help="online noise calculation and conditions")

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = args.mc
    flags.IOVDb.DatabaseInstance= "OFLP200" if args.mc else "CONDBR2"
    from Campaigns.Utils import Campaign
    flags.Input.MCCampaign=Campaign.Unknown
    flags.LAr.doAlign=False
    flags.LAr.doHVCorr=False
    flags.Input.RunNumbers=[args.runnumber]
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
    flags.IOVDb.GlobalTag=defaultConditionsTags.RUN3_MC if args.mc else defaultConditionsTags.RUN3_DATA
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.Common.isOnline = args.online
    if args.pileup >= 0.:
        flags.Calo.Noise.fixedLumiForNoise = args.pileup
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
    cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(AODKey = 'McEventInfo'),sequenceName="AthAlgSeq")
    
    cfg.merge(CaloNoise2NtupleCfg(flags,args.output)) 

    dbstr=None
    if args.database:
        dbstr="sqlite://;schema="+args.database+";dbname=" + flags.IOVDb.DatabaseInstance
                                  
    if args.tag or args.database:
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        folder = "/CALO/Noise/CellNoise" if flags.Common.isOnline else "/LAR/NoiseOfl/CellNoise"
        cfg.merge(addOverride(flags, folder, tag=args.tag, db=dbstr))
        

    sc=cfg.run(1)
    if sc.isSuccess():
        sys.exit(0)
    else:
        sys.exit(1)
