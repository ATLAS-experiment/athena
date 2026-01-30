# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 
def MdtCablMezzAlgCfg(flags, name = "MdtCablMezzAlg", localMezzanineJSON="", localCablingJSON="",  **kwargs):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags, MezzanineJSON=localMezzanineJSON, CablingJSON=localCablingJSON))
    event_algo = CompFactory.MdtCablingJsonDumpAlg(name,**kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonConfig.MuonConfigUtils import executeTest, configureCondTag, SetupMuonStandaloneCA
    from MuonCondTest.MdtCablingTester import SetupArgParser
    
    parser = SetupArgParser()
    
    parser.set_defaults(output="SummaryFile.txt")
    parser.set_defaults(mezzMap="MezzMapping.json")
    parser.set_defaults(cablingMap="MdtCabling.json")
    
    parser.add_argument("--overrideBIS", action='store_true', default=False ,help="Override the BIS cabling in the JSON file")
    parser.add_argument("--localMezzanineJSON", default="", help="Local JSON file containing the mezzanine mapping to use instead of the DB")
    parser.add_argument("--localCablingJSON", default="", help="Local JSON file containing the cabling mapping to use instead of the DB")


   
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
                            name = "MdtCablMezzAlg",
                            localMezzanineJSON=args.localMezzanineJSON,
                            localCablingJSON=args.localCablingJSON,
                            SummaryFile=args.output,
                            OutMezzanineJSON=args.mezzMap,
                            OutCablingJSON=args.cablingMap,
                            insertBISCabling=args.overrideBIS,
                            ))
    if args.overrideBIS:
        print("Will override BIS cabling")
        from  IOVDbSvc.IOVDbSvcConfig import addOverride       
        cfg.merge(addOverride(flags, "/MDT/CABLING/MAP_SCHEMA_JSON", "MDTCablingMapSchemaJSON_RUN3BestKnowledge"))
        cfg.merge(addOverride(flags, "/MDT/CABLING/MEZZANINE_SCHEMA_JSON", "MDTMezMapSchemaJSON_RUN3BestKnowledge"))
    executeTest(cfg)


