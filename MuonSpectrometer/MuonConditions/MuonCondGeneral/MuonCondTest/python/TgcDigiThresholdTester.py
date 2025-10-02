# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def TgcDigtThresholdTestAlgCfg(flags, name="TgcCondDbTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    the_alg = CompFactory.TgcDigtThresholdTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA

    
    parser = SetupArgParser()
    parser.add_argument("--jsonFile", default="TGC_Digitization_energyThreshold.json", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    parser.set_defaults(inputFile=defaultTestFiles.ESD_RUN2_MC)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()
    flags.dump()
   
    cfg = SetupMuonStandaloneCA(flags)
    from MuonConfig.MuonCondAlgConfig import TgcEnergyThresholdCondAlgCfg
    cfg.merge(TgcEnergyThresholdCondAlgCfg(flags, readFromJSON = args.jsonFile))
    cfg.merge(TgcDigtThresholdTestAlgCfg(flags))
  
    executeTest(cfg)