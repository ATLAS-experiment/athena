# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def MdtConditionsTestCfg(flags, name="MdtConditionsTest", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    result.merge(MdtCondDbAlgCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    the_alg = CompFactory.MdtConditionsTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag

   
    
    parser = SetupArgParser()
    parser.add_argument("--LogName", default="LogFile", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    parser.set_defaults(inputFile=defaultTestFiles.ESD_RUN3_MC)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    configureCondTag(flags)
    flags.lock()
    flags.dump()
    
    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(MdtConditionsTestCfg(flags, LogName = args.LogName))
    executeTest(cfg)