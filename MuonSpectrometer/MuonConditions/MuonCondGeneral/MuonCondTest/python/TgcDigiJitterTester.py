# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def TgcDigtJitterTestAlgCfg(flags, name="TgcCondDbTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_alg = CompFactory.TgcDigtJitterTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA
 
    parser = SetupArgParser()
    parser.add_argument("--jsonFile", default="TGC_Digitization_timejitter.json", 
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
    cfg.getService('MessageSvc').setVerbose = ["TgcDigitJitterData"]
    from MuonConfig.MuonCondAlgConfig import TgcDigitJitterCondAlgCfg
    cfg.merge(TgcDigitJitterCondAlgCfg(flags, readFromJSON = args.jsonFile))
    cfg.merge(TgcDigtJitterTestAlgCfg(flags))
    executeTest(cfg)
    