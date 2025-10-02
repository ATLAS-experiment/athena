# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def NSWDcsAlgTest(flags,alg_name="NSWDcsTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import NswDcsDbAlgCfg
    result.merge(NswDcsDbAlgCfg(flags))
    the_alg = CompFactory.NswDcsTestAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag
    
    parser = SetupArgParser()
    parser.add_argument("--LogName", default="LogFile", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
   
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = 1
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    configureCondTag(flags)
    flags.lock()
    flags.dump()

    cfg = SetupMuonStandaloneCA(flags)    
    cfg.merge(NSWDcsAlgTest(flags, LogName = args.LogName))
    
    executeTest(cfg)