# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("-t", "--threads", dest="threads", type=int, help="number of threads", default=1)
    parser.add_argument("-o", "--output", dest="output", default='', help="Text file containing each cabling channel", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--mezzMap", default="", help="External JSON file containing the internal mapping of the mezzanine cards")
    parser.add_argument("--cablingMap", default="", help="External JSON file containing the cabling map of each channel")
    return parser
    
def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        result.merge(PoolReadCfg(flags))
    elif flags.Input.Format == Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        result.merge(ByteStreamReadCfg(flags)) 

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))    
    return result

def MdtCablingTestAlgCfg(flags, 
                        name = "MdtCablingTestAlg", 
                        mezzJSON = "", ### External JSON file containing the mezzanine cards 
                        cablingJSON = "", ### External JSON file containing the channel mapping
                        dumpFile="", ### Dump the cabling into
                        ):
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    result.merge(MdtCondDbAlgCfg(flags))
    result.merge(MDTCablingConfigCfg(flags, MezzanineJSON=mezzJSON, CablingJSON=cablingJSON))
    event_algo = CompFactory.MdtCablingTestAlg(name, DumpMap=dumpFile)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile    
    flags.lock()
    
    cfg = MdtCablingTestAlgCfg(flags,
                               mezzJSON=args.mezzMap,
                               cablingJSON=args.cablingMap,
                               dumpFile=args.output)
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


