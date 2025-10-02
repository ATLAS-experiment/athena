# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag

  
    parser = SetupArgParser()

    parser.set_defaults(inputFile= defaultTestFiles.ESD_RUN3_MC)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Muon.enableNRPC = True
    flags.Concurrency.NumThreads = 1 
    flags.Exec.MaxEvents = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Input.Files = args.inputFile
    configureCondTag(flags)
    flags.lock() 
    flags.dump()
    

    cfg = SetupMuonStandaloneCA(flags)
    
    cfg.merge(RpcCablingTestAlgCfg(flags))  
    if len(args.cablingMap):
        cfg.getCondAlgo("MuonNRPC_CablingAlg").JSONFile = args.cablingMap
    cfg.getService("MessageSvc").debugLimit = 2147483647
    cfg.getService("MessageSvc").verboseLimit = 2147483647
    cfg.getService("MessageSvc").infoLimit = 2147483647
   
    executeTest(cfg)

