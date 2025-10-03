# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def NSWPassivAlgTest(flags,alg_name="NSWPassivAlgTest", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import NswCalibDbAlgCfg
    result.merge(NswCalibDbAlgCfg(flags))
    the_alg = CompFactory.NswPassivationTestAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag
       
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD_RUN3_DATA22
   
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = 1
    configureCondTag(flags)
    flags.lock()
    flags.dump()

    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(NSWPassivAlgTest(flags))
   
    executeTest(cfg)