# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def MicroMegaCablingTestAlgCfg(flags, name = "MMCablingTestAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from MuonConfig.MuonCablingConfig import MmCablingCfg
    from AthenaCommon.Constants import DEBUG
    result = ComponentAccumulator()
    result.merge(MmCablingCfg(flags, JSONFile = "MMGZebraShift.json", OutputLevel = DEBUG ))
    event_algo = CompFactory.MMCablingTestAlg(name, OutputLevel = DEBUG)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag
    parser = SetupArgParser()
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = 1
    flags.Input.Files = defaultTestFiles.ESD_RUN3_DATA22
    configureCondTag(flags)
    flags.lock()   
    
    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(MicroMegaCablingTestAlgCfg(flags))
    flags.dump()
   
    executeTest(cfg)