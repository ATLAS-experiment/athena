
#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

def LArIdCablingTestCfg(flags,isSC=False):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)

    if isSC:
        #Setup SuperCell cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
        result.merge(LArOnOffIdMappingSCCfg(flags))
    else:
        #Setup regular cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
        result.merge(LArOnOffIdMappingCfg(flags))

    result.addEventAlgo(CompFactory.LArIdCablingTest("LArIdCablingTest",isSC=isSC, 
                                                     CablingKey= "LArOnOffIdMapSC" if isSC else "LArOnOffIdMap",
                                                     ))
    return result


if __name__=="__main__":
    import sys,argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
    parser.add_argument("-r","--runnumber",default=0x7fffffff, type=int, help="run number to query the DB")
    parser.add_argument("-d","--database",default="LAR_ONL", help="Database name or sqlite file name")
    parser.add_argument("--SC", action='store_true', help="Work on SuperCells")

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)>0:
        print("ERROR, unhandled argument(s):",leftover)
        sys.exit(-1)
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()

    flags.Input.isMC = False
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.RunNumbers=[args.runnumber]
    flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2023-02"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.lock()

    isSC=args.SC 

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
                                 EventsPerRun      = 1,
                                 FirstEvent        = 1,
                                 InitialTimeStamp  = 0,
                                 TimeStampInterval = 1))




    cfg.merge(LArIdCablingTestCfg(flags,isSC))
    cfg.getService("MessageSvc").errorLimit=5000000
    cfg.run(1)
