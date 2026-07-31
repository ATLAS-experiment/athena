# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


def MuonHitValAlgCfg(flags, name="MuonValAlg", 
                     outFile="BasicMuonTest.root",**kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result = ComponentAccumulator()
    result.merge(setupHistSvcCfg(flags, outFile=outFile, outStream="MUONVALIDSTREAM"))
    kwargs.setdefault("ExtraInputs", [ ( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )])
    the_alg = CompFactory.MuonVal.MuonTesterAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


if __name__ == "__main__":
    from MuonGeoModelTest.testGeoModel import SetupArgParser
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA

    
    args = SetupArgParser().parse_args()
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    from MuonConfig.MuonConfigUtils import prepareInput
    prepareInput(flags, args.inputFile)
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.Exec.MaxEvents = -1
    flags.lock()
    flags.dump(evaluate = True)


    cfg = SetupMuonStandaloneCA(flags)
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))

    cfg.merge(MuonHitValAlgCfg(flags, outFile = args.outRootFile))
    executeTest(cfg)
