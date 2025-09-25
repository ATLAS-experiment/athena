# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def TgcCondDbTestAlgCfg(flags, name="TgcCondDbTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    the_alg = CompFactory.TgcCondDbTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag

    
    parser = SetupArgParser()
    parser.add_argument("--jsonFile", default="TGC_Digitization_2016deadChamber.json", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    parser.set_defaults(inputFile=defaultTestFiles.ESD_RUN2_MC)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1  # Might change this later, but good enough for the moment.
    flags.Exec.MaxEvents = 1
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    configureCondTag(flags)
    flags.lock()
   
    cfg = SetupMuonStandaloneCA(flags)
    from MuonConfig.MuonCondAlgConfig import TgcCondDbAlgCfg
    cfg.merge(TgcCondDbAlgCfg(flags, readFromJSON = args.jsonFile))
    cfg.merge(TgcCondDbTestAlgCfg(flags))
    executeTest(cfg)