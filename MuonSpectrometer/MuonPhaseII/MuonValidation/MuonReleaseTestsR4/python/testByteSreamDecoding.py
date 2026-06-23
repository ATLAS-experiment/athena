# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default= MuonPhaseIITestDefaults.DATA_BS, 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--useSqLite", action="store_true", default = False,
                        help="Schedule whether the phase II geometry shall be used")
    parser.add_argument("--doRdoDecoding", action="store_true", default = False,
                        help="Decode the Rdos to PRD objects afterwards")
    parser.add_argument("--nEvents", help="Number of events to run", type = int ,default = -1)
    parser.add_argument("--skipEvents", help="Number of events to skip", type = int, default = 0)

    return parser
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonGeoModelTestR4.testGeoModel import setupServicesCfg

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    from MuonConfig.MuonConfigUtils import executeTest, configureCondTag
 
    args = SetupArgParser().parse_args()
 

    flags = initConfigFlags()
    flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R3
    flags.GeoModel.SQLiteDB = args.useSqLite
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3    


    flags.Input.Files = args.inputFile
    configureCondTag(flags)

    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Exec.MaxEvents = args.nEvents
    flags.Exec.SkipEvents = args.skipEvents

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True
   
    flags.PerfMon.doFullMonMT = True
    flags.lock()

    cfg = setupServicesCfg(flags)

    cfg.merge(MuonGeoModelCfg(flags))

    if flags.Muon.usePhaseIIGeoSetup:
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        cfg.merge(ActsGeometryContextAlgCfg(flags))

    from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
    cfg.merge(MuonByteStreamDecodersCfg(flags))

    if args.doRdoDecoding:
        from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
        cfg.merge(MuonRDOtoPRDConvertorsCfg(flags))

    executeTest(cfg)