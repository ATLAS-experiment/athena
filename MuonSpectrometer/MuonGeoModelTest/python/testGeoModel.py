
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default=[
                        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"
                        #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"
                        ], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoTag", default="ATLAS-R2-2016-01-02-01", help="Geometry tag to use", choices=["ATLAS-R2-2016-01-02-01",
                                                                                     "ATLAS-R3S-2021-03-02-00"])
    parser.add_argument("--condTag", default="CONDBR2-BLKPA-RUN2-11", help="Conditions tag to use",
                                                                         choices=["OFLCOND-MC16-SDR-RUN2-11",
                                                                                  "OFLCOND-MC23-SDR-RUN3-02",
                                                                                  "CONDBR2-BLKPA-2023-02",
                                                                                  "CONDBR2-BLKPA-RUN2-11"])
    parser.add_argument("--chambers", default=["all"
    ], nargs="+", help="Chambers to check. If string is all, all chambers will be checked")
    parser.add_argument("--outRootFile", default="GeoModelDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--noMdt", help="Disable the Mdts from the geometry", action='store_true', default = False)
    parser.add_argument("--noRpc", help="Disable the Rpcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noTgc", help="Disable the Tgcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noMM", help="Disable the MMs from the geometry", action='store_true', default = False)
    parser.add_argument("--noSTGC", help="Disable the sTgcs from the geometry", action='store_true', default = False)
    
    return parser

def setupHistSvc(flags, out_file="MdtGeoDump.root"):
    result = ComponentAccumulator()
    if len(out_file) == 0: return result
    histSvc = CompFactory.THistSvc(Output=["GEOMODELTESTER DATAFILE='{out_file}', OPT='RECREATE'".format(out_file = out_file)])
    result.addService(histSvc, primary=True)
    return result

def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: return result
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
    flags.lock()
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    cfg.merge(setupHistSvc(flags, out_file = args.outRootFile))
    
    chambToTest =  args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []
    if not args.noMdt:
        cfg.merge(GeoModelMdtTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "B" or ch[0] == "E"], 
                                            dumpSurfaces = False ))
    if not args.noRpc:
        cfg.merge(GeoModelRpcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "B"]))
    if not args.noTgc:
        cfg.merge(GeoModelTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "T"],
                                            ReadoutXML="TgcStripStructure.xml"))

    if not args.noMM:
        cfg.merge(GeoModelMmTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "M"]))    
    
    if not args.noSTGC:
        cfg.merge(GeoModelsTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "S"]))
   
    cfg.merge(GeoModelCscTestCfg(flags))
    
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump(evaluate = True)
    if not cfg.run(1).isSuccess():
        print("Execution failed")
        exit(1)  
