# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def RpcCablingTestAlgCfg(flags, name = "RpcCablingTestAlg", JSONFile="",**kwargs):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import NRPCCablingConfigCfg
    from AthenaCommon.Constants import DEBUG
    result.merge(NRPCCablingConfigCfg(flags, JSONFile = JSONFile, OutputLevel = DEBUG ))
    event_algo = CompFactory.RpcCablingTestAlg(name,**kwargs)
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
    cfg.merge(RpcCablingTestAlgCfg(flags))
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


