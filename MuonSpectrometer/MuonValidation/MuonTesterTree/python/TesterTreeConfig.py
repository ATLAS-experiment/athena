# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TesterTreeAlgCfg(flags, name = "TesterTreeAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonVal.MuonTester.TreeTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__ == "__main__":
    from MuonGeoModelTest.testGeoModel import  SetupArgParser
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    parser = SetupArgParser()
    parser.set_defaults(outRootFile="TesterTreeFile.root")

    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.Exec.MaxEvents = 1
    flags.lock()
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg, executeTest
    cfg.merge(setupHistSvcCfg(flags, outStream="TestStream", outFile = args.outRootFile))
    cfg.merge(TesterTreeAlgCfg(flags))

    executeTest(cfg)