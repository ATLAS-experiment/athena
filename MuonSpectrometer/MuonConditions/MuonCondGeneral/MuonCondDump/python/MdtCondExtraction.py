# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def MdtCondJsonDumpAlgCfg(flags, name="MdtCondJsonDumpAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MdtCondJsonDumpAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()

    args = parser.parse_args()   
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.IOVDb.GlobalTag = args.conditionsTag
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

    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    cfg.merge(MdtCondDbAlgCfg(flags))
    cfg.merge(MdtCondJsonDumpAlgCfg(flags))

    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    cfg.merge(MuonIdHelperSvcCfg(flags))


    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")
