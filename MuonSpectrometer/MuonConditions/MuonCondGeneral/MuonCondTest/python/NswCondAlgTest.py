# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def NSWCondAlgTest(flags,alg_name="NSWCondTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import NswCalibDbAlgCfg
    result.merge(NswCalibDbAlgCfg(flags, processThresholds=True))
    the_alg = CompFactory.NswCondTestAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag
    
    parser = SetupArgParser()
    parser.add_argument("--LogName", default="LogFile", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    parser.add_argument("--isMC", action = 'store_true', default=False)
    parser.set_defaults(inputFile=[])
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD_RUN3_MC if args.isMC else defaultTestFiles.ESD_RUN3_DATA22
   
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = 1
    flags.Muon.Calib.applyMmT0Correction = not args.isMC
    configureCondTag(flags)
    flags.lock()

    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(NSWCondAlgTest(flags, LogName = args.LogName, isMC = flags.Input.isMC))
    executeTest(cfg)