# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def MicroMegaCablingTestAlgCfg(flags, name = "MMCablingTestAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import NswCablingCfg
    from AthenaCommon.Constants import DEBUG
    result.merge(NswCablingCfg(flags, JSONFile = "MMGZebraShift.json", OutputLevel = DEBUG ))
    event_algo = CompFactory.MMCablingTestAlg(name, OutputLevel = DEBUG)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3Data.ESD.pool.root"])
    args = parser.parse_args()

    import os
    os.system("python -m MuonMM_Cabling.zebraShift")

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()   
    
    cfg = MicroMegaCablingTestAlgCfg(flags)  
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


