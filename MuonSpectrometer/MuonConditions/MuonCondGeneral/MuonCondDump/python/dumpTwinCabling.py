# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def MdtToyTwinCablingDumpAlgCfg(flags, name="MdtToyTwinCablingDumpAlg",**kwargs):
    result = ComponentAccumulator() 
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))

    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    kwargs.setdefault("MuonIdHelperSvc", result.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    the_alg = CompFactory.MdtToyTwinCablingDumpAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noTgc=True)
    parser.set_defaults(noRpc=True)
    parser.add_argument("--cablingMap", default="twinMap.json", help="External JSON file containing the cabling map of each channel")
    parser.add_argument("--stationsToTwin",nargs='+', help="Specify the station names for which you want twin tubes",
                        default=[])
    parser.add_argument("--chamberToTwin",nargs='+', help="Specify the station names for which you want twin tubes",
                        default=["BOL4A13M1", "BOL4C13M1"])
    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    cfg.merge(MdtToyTwinCablingDumpAlgCfg(flags, 
                                          stationsToTwin = args.stationsToTwin, 
                                          outCablingJSON = args.cablingMap,
                                          detElIdToTwin = args.chamberToTwin if "all" not in args.chamberToTwin else []))
    
    executeTest(cfg)


