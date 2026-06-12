# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory



class MuonPhaseIITestDefaults:
    ## Particle gun EVGen file (5 -250) GeV spectrum
    EVGEN_PG = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"]
    ### Hits parsed though full R3 ATLAS layout (Only MS hits saved)
    HITS_PG_R3 = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"]
    ### Hits parsed though full R4 ATLAS layout (Only MS hits saved)
    HITS_PG_R4 = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R4SimHits.pool.root"]
    ### Hits parsed through the R3 MS-only ATLAS layout    
    HITS_PG_R3_MSOnly = []
    ### Hits parsed through the R4 MS-only ATLAS layout
    HITS_PG_R4_MSOnly = []
    ### BS file taken in MD3 2025 with a pile-up of >120

    DATA_BS = [f"root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/MuonRecRTT/{fileName}" for fileName in [
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-11._0001.data",
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-11._0002.data",
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-12._0001.data",
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-12._0002.data",
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-20._0001.data",
                            "data26_13p6TeV.00519268.physics_Main.daq.RAW._lb0178._SFO-20._0002.data",
                        ]
    ]
    ### First files taken from https://gitlab.cern.ch/atlas-nextgen/work-package-2.5/SampleProduction/-/blob/master/FileLists/RDO_MU0/R3/999992.PG_DiMuon_Pt10to100.txt
    RDO_R3 = [
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/9c/69/group.det-muon.48959424.EXT0._000002.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/af/b4/group.det-muon.48959424.EXT0._000003.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/d8/a1/group.det-muon.48959424.EXT0._000004.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/ea/62/group.det-muon.48959424.EXT0._000005.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/53/6e/group.det-muon.48959424.EXT0._000006.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/03/66/group.det-muon.48959424.EXT0._000007.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/30/58/group.det-muon.48959424.EXT0._000008.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/95/1e/group.det-muon.48959424.EXT0._000009.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/e7/64/group.det-muon.48959424.EXT0._000010.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/03/cc/group.det-muon.48959424.EXT0._000011.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/49/61/group.det-muon.48959424.EXT0._000012.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/74/ed/group.det-muon.48959424.EXT0._000013.RDO.pool.root",
    ]
    ### First files taken from (https://gitlab.cern.ch/atlas-nextgen/work-package-2.5/SampleProduction/-/blob/master/FileLists/RDO_MU0/R4/999992.PG_DiMuon_Pt10to100.txt)
    RDO_R4 = [   
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/8b/de/group.det-muon.48959425.EXT0._000002.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/4f/ed/group.det-muon.48959425.EXT0._000007.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/3d/10/group.det-muon.48959425.EXT0._000009.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/28/62/group.det-muon.48959425.EXT0._000010.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/5e/26/group.det-muon.48959425.EXT0._000011.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/af/cd/group.det-muon.48959425.EXT0._000012.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/88/76/group.det-muon.48959425.EXT0._000015.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/54/fe/group.det-muon.48959425.EXT0._000016.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/4e/b4/group.det-muon.48959425.EXT0._000017.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/e3/d4/group.det-muon.48959425.EXT0._000022.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/ee/b4/group.det-muon.48959425.EXT0._000023.RDO.pool.root",
        "root://eosatlas.cern.ch:1094//eos/atlas/atlaslocalgroupdisk/dq2/rucio/group/det-muon/75/c9/group.det-muon.48959425.EXT0._000027.RDO.pool.root",
        ]
    ###
    ###     Layout files
    ###

    ### R3 ATLAS layout
    GEODB_R3 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-R3S-2021-03-02-00.db"
    ### R3 MS only layout
    GEODB_R3MSOnly = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-R3S-2021-03-02-00_MSOnly.db"
    ### R3 MTech format - This file format will trigger the setup of the legacy MuonGeoModel and
    ###                   not of the Phase II software    
    GEODB_MTECH = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-R3S-2021-03-02-00_MTech.db"
    ### R4 ATLAS layout
    GEODB_R4 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-P2-RUN4-01-00-00.db"
    ### R4 MS only layout
    GEODB_R4MSOnly = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-P2-RUN4-01-00-00_MSOnly.db"
    ### ITk + Calo + R3-MS ATLAS layout
    GEODB_ITk_R3MS = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-P2-RUN4-01-00-00_R3MS.db"
    #### Only the passive material
    GEODB_TOROID = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/MUON_TOROID.db"

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default= MuonPhaseIITestDefaults.EVGEN_PG, 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default = MuonPhaseIITestDefaults.GEODB_R3, help="GeoModel SqLite file containing the muon geometry.")
    parser.add_argument("--defaultGeoFile", help="Use the  predefined GeoModel files on cvmfs", choices=["NONE", "RUN3", "RUN4", 
                                                                                                         "RUN3MSOnly", "RUN4MSOnly", 
                                                                                                         "ITkR3MS" ], default="NONE")
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
    parser.add_argument("--localMdtMezzJSON", default="", help="")
    parser.add_argument("--localMdtCablingJSON", default="", help="")
    parser.add_argument("--passiveMaterialMaps", default = "", help="Root file with the material maps on the surfaces")
    parser.add_argument("--noPlots", help="Disable the pdf dumps of the geo tester", action='store_true', default = False)
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

def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", localMezzanineJSON="", localCablingJSON="", doPlots=True,**kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags,MezzanineJSON=localMezzanineJSON, CablingJSON=localCablingJSON))
    if not doPlots:
        kwargs["visualizeTubes"] = False
        kwargs["visualizeStaggering"] = False
    the_alg = CompFactory.MuonGMR4.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GeoModelRpcTestCfg(flags, name = "GeoModelRpcTest", doPlots=True,**kwargs):
    result = ComponentAccumulator()
    if not doPlots:
        kwargs["visualizePlanes"] = False
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
    kwargs.setdefault("TestActsSurface", True)
    kwargs.setdefault("plotTgc", flags.Detector.GeometryTGC)
    kwargs.setdefault("plotStgc", flags.Detector.GeometrysTGC)
    kwargs.setdefault("plotMm", flags.Detector.GeometryMM)
    
    the_alg = CompFactory.MuonGMR4.NswGeoPlottingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def configureDefaultTagsCfg(flags):    
    from AthenaCommon.Logging import logging
    log = logging.getLogger('GeometryConfiguration')

    if not flags.GeoModel.SQLiteDB:
        raise ValueError("Default tag configuration only works for SQLite")
    ### For dummy purposes configure the R2 geometry tag such that the job does not crash
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2    
    from AthenaConfiguration.Enums import LHCPeriod
    if flags.GeoModel.Run == LHCPeriod.Run3:   
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    elif flags.GeoModel.Run == LHCPeriod.Run4:
          flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    else:
        raise ValueError(f"Invalid run period {flags.GeoModel.Run}")
    from MuonConfig.MuonConfigUtils import configureCondTag
    configureCondTag(flags)

    log.info(f"Setup {flags.GeoModel.AtlasVersion} geometry loading {flags.GeoModel.SQLiteDBFullPath}")
    log.info(f"Use conditions tag {flags.IOVDb.GlobalTag}")
    

def setupGeoR4TestCfg(args,  flags = None):
    
    if flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Exec.MaxEvents = args.nEvents
    flags.Exec.SkipEvents = args.skipEvents
    from os import path, system, listdir
    flags.Input.Files = []
    ### Assemble all files in a directory or all files not having the suffix txt conf. 
    ### The latter are interpreted as file lists
    for fileArg in args.inputFile:
        if path.isdir(fileArg):
            flags.Input.Files += [ "{dir}/{file}".format(dir=fileArg, file=y) for y in listdir(fileArg) ]
        else:
            if fileArg[fileArg.rfind(".")+1 :]not in ["txt", "conf"]:
                    flags.Input.Files+=[fileArg]
            else:
                with open(fileArg) as inStream:
                   #Check if the input is a string of comma separated files, and if it is, split it into a list
                   if isinstance(inStream, str) and "," in inStream:
                       flags.Input.Files += inStream.split(",")
                   else:
                      flags.Input.Files+=[ line.strip() for line in inStream if line[0]!='#'] 

    flags.Exec.FPE= 500
    flags.Exec.EventPrintoutInterval = 500
    
    if args.defaultGeoFile == "RUN3":
        flags.GeoModel.SQLiteDBFullPath =  MuonPhaseIITestDefaults.GEODB_R3
    elif args.defaultGeoFile == "RUN4":
        flags.GeoModel.SQLiteDBFullPath =  MuonPhaseIITestDefaults.GEODB_R4
    elif args.defaultGeoFile == "RUN3MSOnly":
        flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R3MSOnly
    elif args.defaultGeoFile == "RUN4MSOnly":
        flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4MSOnly
    elif args.defaultGeoFile == "ITkR3MS":
        flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_ITk_R3MS
    elif args.geoModelFile.startswith("root://"):
        if not path.exists("Geometry/{geoTag}.db".format(geoTag=args.geoTag)):
            print ("Copy geometry file from EOS {source}".format(source = args.geoModelFile))
            system("mkdir Geometry/")
            system("xrdcp {source} Geometry/{geoTag}.db".format(source = args.geoModelFile,
                                                                geoTag=args.geoTag))
                                
        args.geoModelFile = "Geometry/{geoTag}.db".format(geoTag=args.geoTag)
    else:
        flags.GeoModel.SQLiteDBFullPath = args.geoModelFile

    flags.GeoModel.SQLiteDB = True
    configureDefaultTagsCfg(flags)

    if args.passiveMaterialMaps:        
        flags.Muon.trackGeometryMaterialMap = args.passiveMaterialMaps
  
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
    #### Flags from ACTS are not defined in AthSimulation.
    try:
        flags.Acts.TrackingGeometry.UseBlueprint = True
    except AttributeError: pass
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
    cfg.getService("MessageSvc").errorLimit = 10000000

    return flags, cfg


if __name__=="__main__":
    args = SetupArgParser().parse_args()
    flags, cfg = setupGeoR4TestCfg(args)  
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    cfg.merge(setupHistSvcCfg(flags, outFile = args.outRootFile, outStream="GEOMODELTESTER"))
    chambToTest =  args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []
    chambToExclude = args.excludedChambers
    
    ### Ensure consistent translation of the geometry
    if flags.Muon.usePhaseIIGeoSetup:
        cfg.getCondAlgo("MuonDetectorCondAlg").checkGeo = True
        cfg.getCondAlgo("MuonDetectorCondAlg").dumpGeo = True
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
                                         localMezzanineJSON=args.localMdtMezzJSON,
                                         localCablingJSON=args.localMdtCablingJSON,
                                         ReadoutSideXML="ReadoutSides.xml",
                                         doPlots=not args.noPlots,
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
                                         doPlots=not args.noPlots,
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
    from MuonConfig.MuonConfigUtils import executeTest
    executeTest(cfg)
