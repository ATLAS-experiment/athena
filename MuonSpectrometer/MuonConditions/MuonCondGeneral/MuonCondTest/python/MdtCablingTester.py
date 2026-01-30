# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def SetupArgParser():
    from argparse import ArgumentParser
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    parser = ArgumentParser()
    parser.add_argument("-t", "--threads", dest="threads", type=int, help="number of threads", default=1)
    parser.add_argument("-o", "--output", dest="output", default='', help="Text file containing each cabling channel", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=[], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoTag", default=defaultGeometryTags.RUN3, help="Geometry tag to use", choices=[defaultGeometryTags.RUN2_BEST_KNOWLEDGE ,
                                                                                                           defaultGeometryTags.RUN3])
    parser.add_argument("--mezzMap", default="", help="External JSON file containing the internal mapping of the mezzanine cards")
    parser.add_argument("--cablingMap", default="", help="External JSON file containing the cabling map of each channel")
    return parser
    
def MdtCablingTestAlgCfg(flags, 
                        name = "MdtCablingTestAlg", 
                        mezzJSON = "", ### External JSON file containing the mezzanine cards 
                        cablingJSON = "", ### External JSON file containing the channel mapping
                        dumpFile="", ### Dump the cabling into
                        ):

    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    result.merge(MdtCondDbAlgCfg(flags))
    result.merge(MDTCablingConfigCfg(flags, MezzanineJSON=mezzJSON, CablingJSON=cablingJSON))
    event_algo = CompFactory.MdtCablingTestAlg(name, DumpMap=dumpFile)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag

    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Exec.MaxEvents = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Output.ESDFileName = args.output

    flags.Input.Files = args.inputFile
    if not flags.GeoModel.AtlasVersion:
        flags.GeoModel.AtlasVersion = args.geoTag

    configureCondTag(flags)

 
    
    flags.lock()
    flags.dump()

    

    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(MdtCablingTestAlgCfg(flags,
                               mezzJSON=args.mezzMap,
                               cablingJSON=args.cablingMap,
                               dumpFile=args.output))
    executeTest(cfg)

