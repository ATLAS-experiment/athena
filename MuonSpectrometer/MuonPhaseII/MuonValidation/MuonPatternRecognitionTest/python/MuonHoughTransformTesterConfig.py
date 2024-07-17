# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHoughTransformTesterCfg(flags, name = "MuonHoughTransformTester", **kwargs):
    result = ComponentAccumulator()
    containerNames = []
    ## If the tester runs on MC add the truth information
    if flags.Input.isMC:
        if flags.Detector.EnableMDT: containerNames+=["xMdtSimHits"]
        if flags.Detector.EnableMM: containerNames+=["xMmSimHits"]
        if flags.Detector.EnableRPC: containerNames+=["xRpcSimHits"]
        if flags.Detector.EnableTGC: containerNames+=["xTgcSimHits"]
        if flags.Detector.EnablesTGC: containerNames+=["xStgcSimHits"] 
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
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2023-02")
    parser.set_defaults(inputFile=[
                                    #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"
                                    ])
    parser.set_defaults(eventPrintoutLevel = 500)
    parser.add_argument("--displayFailedSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)
    parser.add_argument("--displayGoodSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)


    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,
                                    out_stream="MuonEtaHoughTransformTest"))

    if flags.Input.isMC:
        from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
        cfg.merge(MuonSimHitToMeasurementCfg(flags))
    else:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
        cfg.merge(MuonByteStreamDecodersCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
        cfg.merge(MuonRDOtoPRDConvertorsCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                          drawDisplayFailed =args.displayFailedSeeds,
                                          drawDisplaySuccss = args.displayGoodSeeds))
    cfg.merge(PerfMonMTSvcCfg(flags))
    
    
    
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg, MuonSegmentFinderAlgCfg, MuonSegmentCnvAlgCfg
    ### Build segments from the leagcy chain
    cfg.merge(MuonLayerHoughAlgCfg(flags))
    cfg.merge(MuonSegmentFinderAlgCfg(flags,
                                      NSWSegmentCollectionName=""))

    cfg.merge(MuonSegmentCnvAlgCfg(flags))
    from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg
    cfg.merge(MuPatTrackBuilderCfg(flags))
    ### What happens if you parse the R4 patterns to the legacy chain?
    from MuonPatternCnv.MuonPatternCnvConfig import MuonPatternCnvAlgCfg
    cfg.merge(MuonPatternCnvAlgCfg(flags,
                                   PatternCombiKey="R4Patterns",
                                   HoughDataPerSecKey="R4HoughDataPerSec"))
    cfg.merge(MuonSegmentFinderAlgCfg(flags,
                                      name="MuonSegmentFinderR4Pattern",
                                      MuonLayerHoughCombisKey="R4Patterns",
                                      SegmentCollectionName="TrackMuonSegmentsFromR4",
                                      NSWSegmentCollectionName=""))
    cfg.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlgFromR4",
                                   SegmentContainerName="TrackMuonSegmentsFromR4",
                                   xAODContainerName="MuonSegmentsFromR4"))
    
    cfg.merge(MuPatTrackBuilderCfg(flags, name="TrackBuildingFromR4",
                                   MuonSegmentCollection = "TrackMuonSegmentsFromR4",
                                   SpectrometerTrackOutputLocation="MuonTracksFromR4"))

    # cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))

    executeTest(cfg, args.nEvents)
    
