# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator   

def MuonABLineJsonDumpAlgCfg(flags, name = "MuonABLineJsonDumpAlg", **kwargs):    
    result = ComponentAccumulator()
    event_algo = CompFactory.MuonABLineJsonDumpAlg(name,**kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    parser = SetupArgParser()
    parser.set_defaults(output="ForkLiftTruckDrivingIsFun.json")
    parser.set_defaults(inputFile = defaultTestFiles.ESD_RUN3_DATA22)  
    parser.add_argument("--alignJsonFile", 
                        help="External JSON file parsed to the Alignment algorithm", 
                        default="")
    args = parser.parse_args()   
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
    elif flags.Input.Format == Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags)) 

    from MuonConfig.MuonGeometryConfig import MuonAlignmentCondAlgCfg
    cfg.merge(MuonAlignmentCondAlgCfg(flags, readFromJSON = args.alignJsonFile))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    cfg.merge(MuonIdHelperSvcCfg(flags))

    cfg.merge(MuonABLineJsonDumpAlgCfg(flags, OutFile=args.output))
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


