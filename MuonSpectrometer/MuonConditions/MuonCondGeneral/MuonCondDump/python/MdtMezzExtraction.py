# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 
def MdtCablMezzAlgCfg(flags, name = "MdtCablMezzAlg", **kwargs):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags))
    event_algo = CompFactory.MdtCablingJsonDumpAlg(name,**kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest, configureCondTag, SetupMuonStandaloneCA
    
    parser = SetupArgParser()
    parser.set_defaults(output="SummaryFile.txt")
    parser.set_defaults(mezzMap="MezzMapping.json")
    parser.set_defaults(cablingMap="MdtCabling.json")
   
    args = parser.parse_args()   
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = 1
    flags.Input.Files = args.inputFile
    if not flags.GeoModel.AtlasVersion:
      flags.GeoModel.AtlasVersion = args.geoTag
    configureCondTag(flags)
    flags.lock()
    flags.dump(evaluate=True)

    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge( MdtCablMezzAlgCfg(flags,
                            SummaryFile=args.output,
                            OutMezzanineJSON=args.mezzMap,
                            OutCablingJSON=args.cablingMap))
    executeTest(cfg)


