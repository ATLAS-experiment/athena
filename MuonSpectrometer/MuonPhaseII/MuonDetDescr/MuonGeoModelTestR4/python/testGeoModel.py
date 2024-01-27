# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--geoTag", default="ATLAS-R3S-2021-03-02-00", help="Geometry tag to use", choices=["ATLAS-R3S-2021-03-02-00",
                                                                                                            "ATLAS-P2-RUN4-01-00-00"])
    parser.add_argument("--condTag", default="OFLCOND-MC23-SDR-RUN3-02", help="Conditions tag to use",
                                                                         choices= ["OFLCOND-MC23-SDR-RUN3-02"])
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default ="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/muonsOnlyR4WMDT.db", help="GeoModel SqLite file containing the muon geometry.")
    parser.add_argument("--chambers", default=["all"], nargs="+", help="Chambers to check. If string is all, all chambers will be checked")
    parser.add_argument("--outRootFile", default="MdtGeoDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--nEvents", help="Number of events to rum", type = int ,default = 1)
    parser.add_argument("--noMdt", help="Disable the Mdts from the geometry", action='store_true', default = False)
    parser.add_argument("--noRpc", help="Disable the Rpcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noTgc", help="Disable the Tgcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noMM", help="Disable the MMs from the geometry", action='store_true', default = False)
    parser.add_argument("--noSTGC", help="Disable the sTgcs from the geometry", action='store_true', default = False)
    return parser

def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        result.merge(PoolReadCfg(flags))

    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    return result

def setupHistSvcCfg(flags, out_file="MdtGeoDump.root", out_stream="GEOMODELTESTER"):
    result = ComponentAccumulator()
    if len(out_file) == 0: return result
    histSvc = CompFactory.THistSvc(Output=[f"{out_stream} DATAFILE='{out_file}', OPT='RECREATE'"])
    result.addService(histSvc, primary=True)
    return result


def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    the_alg = CompFactory.MuonGMR4.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelRpcTestCfg(flags, name = "GeoModelRpcTest", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    the_alg = CompFactory.MuonGMR4.GeoModelRpcTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelTgcTestCfg(flags, name = "GeoModelTgcTest", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonGMR4.GeoModelTgcTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelsTgcTestCfg(flags, name = "GeoModelsTgcTest", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    the_alg = CompFactory.MuonGMR4.GeoModelsTgcTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonChamberToolTestCfg(flags, name="MuonChamberToolTest", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    from ActsGeometry.DetectorVolumeSvcCfg import DetectorVolumeSvcCfg
    kwargs.setdefault("DetectorVolumeSvc", result.getPrimaryAndMerge(DetectorVolumeSvcCfg(flags)))
    the_alg = CompFactory.MuonGMR4.MuonChamberToolTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)    
    return result

def setupGeoR4TestCfg(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Input.isMC = args.condTag.find("OFLCOND") != -1
    flags.Input.Files = args.inputFile 
    flags.Exec.FPE= 500
    from os import path, system
    if args.geoModelFile.startswith("root://"):
        if not path.exists("MuonGeometryDB.db"):
            print ("Copy geometry file from EOS {source}".format(source = args.geoModelFile))
            system("xrdcp {source} MuonGeometryDB.db".format(source = args.geoModelFile))
        args.geoModelFile = "MuonGeometryDB.db"
    
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    if args.geoModelFile:
        flags.GeoModel.SQLiteDB = True
        from AtlasGeoModel import CommonGeoDB
        CommonGeoDB.SetupLocalSqliteGeometryDb(args.geoModelFile,args.geoTag)
    
    flags.Detector.GeometryBpipe = False
    ### Inner detector
    flags.Detector.GeometryBCM = False
    flags.Detector.GeometryPixel = False
    flags.Detector.GeometrySCT = False
    flags.Detector.GeometryTRT = False
    ### ITK
    flags.Detector.GeometryPLR = False
    flags.Detector.GeometryBCMPrime = False
    flags.Detector.GeometryITkPixel = False
    flags.Detector.GeometryITkStrip = False
    ### HGTD
    flags.Detector.GeometryHGTD = False
    ### Calorimeter
    flags.Detector.GeometryLAr = False
    flags.Detector.GeometryTile = False
    flags.Detector.GeometryMBTS = False
    flags.Detector.GeometryCalo = False
    ### Muon spectrometer
    flags.Detector.GeometryCSC = False
    flags.Detector.GeometrysTGC = not args.noSTGC
    flags.Detector.GeometryMM = not args.noMM
    flags.Detector.GeometryTGC = not args.noTgc
    flags.Detector.GeometryRPC = not args.noRpc
    flags.Detector.GeometryMDT = not args.noMdt

    flags.Muon.setupGeoModelXML = True

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True
   

    flags.lock()
    flags.dump(evaluate = True)


    cfg = setupServicesCfg(flags)
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))
    return flags, cfg

def executeTest(cfg, num_events = 1):
    DetDescCnvSvc = cfg.getService("DetDescrCnvSvc")
    DetDescCnvSvc.IdDictFromRDB = False
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.10.00.xml"
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.09.03.xml"
    
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run(num_events).isSuccess(): exit(1)
if __name__=="__main__":
    args = SetupArgParser().parse_args()
    flags, cfg = setupGeoR4TestCfg(args)  
    cfg.merge(setupHistSvcCfg(flags, out_file = args.outRootFile))
    chambToTest =  args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []
    
    if flags.Detector.GeometryMDT:
        cfg.merge(GeoModelMdtTestCfg(flags, 
                                     TestStations = [ch for ch in chambToTest if ch[0] == "B" or ch[0] == "E"],
                                     ReadoutSideXML="ReadoutSides.xml"))

    if flags.Detector.GeometryRPC: 
        cfg.merge(GeoModelRpcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "B"]))

    if flags.Detector.GeometryTGC: 
        cfg.merge(GeoModelTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "T"]))
    
    if flags.Detector.GeometrysTGC: 
        cfg.merge(GeoModelsTgcTestCfg(flags, TestStations = [ch for ch in chambToTest if ch[0] == "S"]))
    
    executeTest(cfg, num_events = args.nEvents)
