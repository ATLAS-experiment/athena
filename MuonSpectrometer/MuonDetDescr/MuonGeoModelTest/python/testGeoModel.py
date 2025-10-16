
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles

    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default=defaultTestFiles.EVNT, 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoTag", default=defaultGeometryTags.RUN3, help="Geometry tag to use", choices=[defaultGeometryTags.RUN2_BEST_KNOWLEDGE ,
                                                                                                           defaultGeometryTags.RUN3])
    parser.add_argument("--condTag", default=defaultConditionsTags.RUN3_MC, help="Conditions tag to use",
                                                                            choices=[defaultConditionsTags.RUN3_MC,
                                                                                     defaultConditionsTags.RUN3_DATA,
                                                                                     defaultConditionsTags.RUN2_DATA,
                                                                                     defaultConditionsTags.RUN2_MC])
    parser.add_argument("--chambers", default=["all"
    ], nargs="+", help="Chambers to check. If string is all, all chambers will be checked")
    parser.add_argument("--excludedChambers", default=[], nargs="+", help="Chambers to exclude. If string contains 'none', all chambers will be checked. Note: adding a chamber to --excludedChambers will overwrite it being in --chambers.")
    parser.add_argument("--outRootFile", default="LegacyGeoModelDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--noMdt", help="Disable the Mdts from the geometry", action='store_true', default = False)
    parser.add_argument("--noRpc", help="Disable the Rpcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noTgc", help="Disable the Tgcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noMM", help="Disable the MMs from the geometry", action='store_true', default = False)
    parser.add_argument("--noSTGC", help="Disable the sTgcs from the geometry", action='store_true', default = False)
    
    return parser


def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: return result
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags))
    the_alg = CompFactory.MuonGM.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def GeoModelRpcTestCfg(flags,name = "GeoModelRpcTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryRPC: return result
    the_alg = CompFactory.MuonGM.GeoModelRpcTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def GeoModelTgcTestCfg(flags,name = "GeoModelTgcTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryTGC: return result
    the_alg = CompFactory.MuonGM.GeoModelTgcTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result
def GeoModelMmTestCfg(flags,name = "GeoModelMmTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMM: return result
    the_alg = CompFactory.MuonGM.GeoModelMmTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def GeoModelsTgcTestCfg(flags, name = "GeoModelsTgcTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometrysTGC: return result
    the_alg = CompFactory.MuonGM.GeoModelsTgcTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def GeoModelCscTestCfg(flags, name = "GeoModelCscTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryCSC: return result
    the_alg = CompFactory.MuonGM.GeoModelCscTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
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
    flags.dump(evaluate = True)
    
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg, SetupMuonStandaloneCA
    cfg = SetupMuonStandaloneCA(flags)
    cfg.merge(setupHistSvcCfg(flags, outFile = args.outRootFile, outStream="GEOMODELTESTER"))
    
    chambToTest =  args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []
    chambToExclude = args.excludedChambers
    if not args.noMdt:
        cfg.merge(GeoModelMdtTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "B" or ch[0] == "E"],
                                            ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B" or ch[0] == "E"]))
    if not args.noRpc:
        cfg.merge(GeoModelRpcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "B"],
                                      ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B"]))
    if not args.noTgc:
        cfg.merge(GeoModelTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "T"],
                                            ExcludeStations = [ch for ch in chambToExclude if ch[0] == "T"],
                                            ReadoutXML="TgcStripStructure.xml"))

    if not args.noMM:
        cfg.merge(GeoModelMmTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "M"],
                                    ExcludeStations = [ch for ch in chambToExclude if ch[0] == "M"]))    
    
    if not args.noSTGC:
        cfg.merge(GeoModelsTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "S"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "S"]))
   
    cfg.merge(GeoModelCscTestCfg(flags))
    
    executeTest(cfg)