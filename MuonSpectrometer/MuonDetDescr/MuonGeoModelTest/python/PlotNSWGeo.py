# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator   

def NSWGeoPlottingAlgCfg(flags, name = "NSWGeoPlottingAlg", **kwargs):
    result = ComponentAccumulator()
    event_algo = CompFactory.MuonGM.NSWGeoPlottingAlg(name, **kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from AthenaCommon.TestDefaults import defaultTestFiles
    parser = SetupArgParser()
    parser.set_defaults(inputFile=defaultTestFiles.EVNT)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.Muon.applyMMPassivation = True
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    flags.lock()
    #### 
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    
    cfg.merge(NSWGeoPlottingAlgCfg(flags))

    msgService = cfg.getService('MessageSvc')
  
    cfg.printConfig(withDetails=True, summariseProps=True)

    flags.dump()
    
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")
