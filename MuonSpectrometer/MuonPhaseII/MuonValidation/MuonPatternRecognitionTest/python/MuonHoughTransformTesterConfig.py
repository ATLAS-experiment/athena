# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHoughTransformTesterCfg(flags, name = "MuonHoughTransformTester", **kwargs):
    result = ComponentAccumulator()
    containerNames = []
    if flags.Detector.EnableMDT:
        containerNames+=["xMdtSimHits"]
    if flags.Detector.EnableMM:
        containerNames+=["xMmSimHits"]
    if flags.Detector.EnableRPC:
        containerNames+=["xRpcSimHits"]
    if flags.Detector.EnableTGC:
        containerNames+=["xTgcSimHits"]
    if flags.Detector.EnablesTGC:
        containerNames+=["xStgcSimHits"] 
    kwargs.setdefault("SimHitKeys", containerNames)
    theAlg = CompFactory.MuonValR4.MuonHoughTransformTester(name, **kwargs)    

    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])
    parser.set_defaults(eventPrintoutLevel = 500)
    parser.add_argument("--displayFailedSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)
    parser.add_argument("--displayGoodSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)


    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,out_stream="MuonEtaHoughTransformTest"))

    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    cfg.merge(MuonSimHitToMeasurementCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                       drawDisplayFailed =args.displayFailedSeeds,
                                       drawDisplaySuccss = args.displayGoodSeeds))
    cfg.merge(PerfMonMTSvcCfg(flags))
    # cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))

    # output spam reduction
    cfg.getService("AthenaHiveEventLoopMgr").EventPrintoutInterval=500


    executeTest(cfg, args.nEvents)
    
