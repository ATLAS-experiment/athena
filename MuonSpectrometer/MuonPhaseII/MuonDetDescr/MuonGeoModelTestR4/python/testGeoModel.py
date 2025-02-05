# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def geoModelFileDefault(useR4Layout = False):
    # If this is changed, remember to also test with other dependent tests 
    # e.g. run ctest with ActsEventCnv
    if useR4Layout: 
        return  "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R4-MUONTEST.db"
    return "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-R3S-2021-03-02-00.db"

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--geoTag", default="ATLAS-R3S-2021-03-02-00", help="Geometry tag to use", choices=["ATLAS-R3S-2021-03-02-00",
                                                                                                            "ATLAS-P2-RUN4-01-00-00"])
    parser.add_argument("--condTag", default="OFLCOND-MC23-SDR-RUN3-07", help="Conditions tag to use",
                                                                         choices= ["OFLCOND-MC23-SDR-RUN3-07", "CONDBR2-BLKPA-2023-03", "OFLCOND-MC21-SDR-RUN4-01"])
    parser.add_argument("--inputFile", "-i", default=[
                                                      #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                                      "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"
                                                      ], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default = geoModelFileDefault(), help="GeoModel SqLite file containing the muon geometry.")
    parser.add_argument("--chambers", default=["all"], nargs="+", help="Chambers to check. If string is all, all chambers will be checked")
    parser.add_argument("--excludedChambers", default=[], nargs="+", help="Chambers to exclude. If string contains 'none', all chambers will be checked. Note: adding a chamber to --excludedChambers will overwrite it being in --chambers.")
    parser.add_argument("--outRootFile", default="NewGeoModelDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--nEvents", help="Number of events to run", type = int ,default = 1)
    parser.add_argument("--skipEvents", help="Number of events to skip", type = int, default = 0)
    parser.add_argument("--noMdt", help="Disable the Mdts from the geometry", action='store_true', default = False)
    parser.add_argument("--noRpc", help="Disable the Rpcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noTgc", help="Disable the Tgcs from the geometry", action='store_true', default = False)
    parser.add_argument("--noMM", help="Disable the MMs from the geometry", action='store_true', default = False)
    parser.add_argument("--noSTGC", help="Disable the sTgcs from the geometry", action='store_true', default = False)
    parser.add_argument("--eventPrintoutLevel", type=int, help="Interval of event heartbeat printouts from the loop manager", default = 1)
    return parser

def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        result.merge(PoolReadCfg(flags))
    elif flags.Input.Format == Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        result.merge(ByteStreamReadCfg(flags)) 

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    return result

def setupHistSvcCfg(flags, outFile="MdtGeoDump.root", outStream="GEOMODELTESTER"):
    result = ComponentAccumulator()
    if len(outFile) == 0: return result
    histSvc = CompFactory.THistSvc(Output=[f"{outStream} DATAFILE='{outFile}', OPT='RECREATE'"])
    result.addService(histSvc, primary=True)
    return result


def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonGMR4.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelRpcTestCfg(flags, name = "GeoModelRpcTest", **kwargs):
    result = ComponentAccumulator()
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
    the_alg = CompFactory.MuonGMR4.GeoModelsTgcTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelMmTestCfg(flags, name = "GeoModelMmTest", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonGMR4.GeoModelMmTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def NswGeoPlottingAlgCfg(flags, name="NswGeoPlotting", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("TestActsSurface", False)
    the_alg = CompFactory.MuonGMR4.NswGeoPlottingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def setupGeoR4TestCfg(args,  flags = None):
    
    if flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Exec.MaxEvents = args.nEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.Input.isMC = args.condTag.find("OFLCOND") != -1
    from os import path, system, listdir
    inFiles = [x for x in args.inputFile if not path.isdir(x)] + \
              [ "{dir}/{file}".format(dir=x, file=y)  for x in args.inputFile if path.isdir(x) for y in listdir(x) ]
    flags.Input.Files = inFiles 
    flags.Exec.FPE= 500
    flags.Exec.EventPrintoutInterval = args.eventPrintoutLevel
    
    if args.geoModelFile.startswith("root://"):
        if not path.exists("Geometry/{geoTag}.db".format(geoTag=args.geoTag)):
            print ("Copy geometry file from EOS {source}".format(source = args.geoModelFile))
            system("mkdir Geometry/")
            system("xrdcp {source} Geometry/{geoTag}.db".format(source = args.geoModelFile,
                                                                geoTag=args.geoTag))
                                
        args.geoModelFile = "Geometry/{geoTag}.db".format(geoTag=args.geoTag)
    print ("Use geometry file: {geoFile}".format(geoFile = args.geoModelFile))
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.GeoModel.SQLiteDB = True
    flags.GeoModel.SQLiteDBFullPath = args.geoModelFile
    
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
    if args.noSTGC:
        flags.Detector.GeometrysTGC = False
    if args.noMM:
        flags.Detector.GeometryMM = False
    if args.noTgc:
        flags.Detector.GeometryTGC = False
    if args.noRpc:
        flags.Detector.GeometryRPC = False    
    if args.noMdt:
        flags.Detector.GeometryMDT = False

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True
    #flags.PerfMon.doFullMonMT = True
   
    flags.lock()
    flags.dump(evaluate = True)
    cfg = setupServicesCfg(flags)

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))


    if not flags.Muon.usePhaseIIGeoSetup:
        print ("WARNING: New Muon plugin is not part of the Geometry file {geoDBFile}".format(geoDBFile=args.geoModelFile))
    else:
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        cfg.merge(ActsGeometryContextAlgCfg(flags))

    cfg.getService("MessageSvc").verboseLimit = 10000000
    cfg.getService("MessageSvc").debugLimit = 10000000

    return flags, cfg

def executeTest(cfg):
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run().isSuccess(): exit(1)

if __name__=="__main__":
    args = SetupArgParser().parse_args()
    flags, cfg = setupGeoR4TestCfg(args)  
    cfg.merge(setupHistSvcCfg(flags, outFile = args.outRootFile))
    chambToTest =  args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []
    chambToExclude = args.excludedChambers
    
    ### Ensure consistent translation of the geometry
    if flags.Muon.usePhaseIIGeoSetup:
        cfg.getCondAlgo("MuonDetectorManagerCondAlg").checkGeo = True
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
        cfg.merge(TrackingGeometryCondAlgCfg(flags))
    
    
    cfg.getService("MessageSvc").setVerbose = []

    

    if flags.Detector.GeometryMDT:
        if not flags.Muon.usePhaseIIGeoSetup:
            from MuonGeoModelTest.testGeoModel import GeoModelMdtTestCfg as LegacyTestCfg
            cfg.merge(LegacyTestCfg(flags,
                                         TestStations = [ch for ch in chambToTest if ch[0] == "B" or ch[0] == "E"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B" or ch[0] == "E"]))
        else:
            cfg.merge(GeoModelMdtTestCfg(flags, 
                                         TestStations = [ch for ch in chambToTest if ch[0] == "B" or ch[0] == "E"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B" or ch[0] == "E"],
                                         ReadoutSideXML="ReadoutSides.xml",
                                         ExtraInputs=[( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )]))

    if flags.Detector.GeometryRPC: 
        if not flags.Muon.usePhaseIIGeoSetup:
            from MuonGeoModelTest.testGeoModel import GeoModelRpcTestCfg as LegacyTestCfg
            cfg.merge(LegacyTestCfg(flags,
                                         TestStations = [ch for ch in chambToTest if ch[0] == "B"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B"]))
        else:
            cfg.merge(GeoModelRpcTestCfg(flags, 
                                         TestStations = [ch for ch in chambToTest if ch[0] == "B"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "B"],
                                         ExtraInputs=[( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )]))

    if flags.Detector.GeometryTGC:
        if not flags.Muon.usePhaseIIGeoSetup:
            from MuonGeoModelTest.testGeoModel import GeoModelTgcTestCfg as LegacyTestCfg
            cfg.merge(LegacyTestCfg(flags,
                                         TestStations = [ch for ch in chambToTest if ch[0] == "T"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "T"]))
        else:
            cfg.merge(GeoModelTgcTestCfg(flags, 
                                         TestStations = [ch for ch in chambToTest if ch[0] == "T"],
                                         ExcludeStations = [ch for ch in chambToExclude if ch[0] == "T"],
                                         ExtraInputs=[( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )]))

    if flags.Detector.GeometryMM:
        if not flags.Muon.usePhaseIIGeoSetup:
            from MuonGeoModelTest.testGeoModel import GeoModelMmTestCfg as LegacyTestCfg
            cfg.merge(LegacyTestCfg(flags,
                                        TestStations = [ch for ch in chambToTest if ch[0] == "M"],
                                        ExcludeStations = [ch for ch in chambToExclude if ch[0] == "M"])) 
        else:

            cfg.merge(GeoModelMmTestCfg(flags, 
                                        TestStations = [ch for ch in chambToTest if ch[0] == "M"],
                                        ExcludeStations = [ch for ch in chambToExclude if ch[0] == "M"],
                                        ExtraInputs=[( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )]))
    
    if flags.Detector.GeometrysTGC:
        if not flags.Muon.usePhaseIIGeoSetup:
            from MuonGeoModelTest.testGeoModel import GeoModelsTgcTestCfg as LegacyTestCfg
            cfg.merge(LegacyTestCfg(flags,
                                        TestStations = [ch for ch in chambToTest if ch[0] == "S"],
                                        ExcludeStations = [ch for ch in chambToExclude if ch[0] == "S"]))  
        else:
            cfg.merge(GeoModelsTgcTestCfg(flags, 
                                          TestStations = [ch for ch in chambToTest if ch[0] == "S"],
                                          ExcludeStations = [ch for ch in chambToExclude if ch[0] == "S"],
                                          ExtraInputs=[( 'MuonGM::MuonDetectorManager' , 'ConditionStore+MuonDetectorManager' )]))
    
    executeTest(cfg)
