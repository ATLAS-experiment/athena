# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def RpcCablingTestAlgCfg(flags, name = "RpcCablingTestAlg", JSONFile="",**kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCablingConfig import NRPCCablingConfigCfg
    from AthenaCommon.Constants import DEBUG
    result = ComponentAccumulator()
    result.merge(NRPCCablingConfigCfg(flags, JSONFile = JSONFile, OutputLevel = DEBUG ))
    event_algo = CompFactory.Muon.RpcCablingTestAlg(name, OutputLevel = DEBUG, **kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3MC.ESD.pool.root"])
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Muon.enableNRPC = True
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()   

    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    
    cfg.merge( RpcCablingTestAlgCfg(flags))  
    if len(args.cablingMap):
        cfg.getCondAlgo("MuonNRPC_CablingAlg").JSONFile = args.cablingMap
    cfg.getService("MessageSvc").debugLimit = 2147483647
    cfg.getService("MessageSvc").verboseLimit = 2147483647
    cfg.getService("MessageSvc").infoLimit = 2147483647

    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


