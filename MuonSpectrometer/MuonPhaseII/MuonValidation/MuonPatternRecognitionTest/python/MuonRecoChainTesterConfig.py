# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaborationation

if __name__=="__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="RecoChainTester.root")
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"])
    parser.add_argument("--monitorPlots", action='store_true', default=False, 
                        help="Setup monitoring plots of the pattern recognition")
    parser.add_argument("--runVtune", 
                        help="runs VTune profiler service for the muon hough alg", action='store_true', default = False)
    parser.add_argument("--noPerfMon", help="If set to true, full perfmonMT is enabled",
                        default=False, action='store_true')
    parser.add_argument("--houghR4", help="Schedules the R4 pattern -> legacy segment -> legacy track chain",
                        action="store_true", default = False)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"

    flags, cfg = setupGeoR4TestCfg(args,flags)
    

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))
    
    ### Build segments from the leagcy chain
    from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg
    cfg.merge(LegacyMuonRecoChainCfg(flags))
    ### Setup the new chain
    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))    
    cfg.merge(MuonSegmentFittingAlgCfg(flags))
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg
    cfg.merge(MSTrackFinderAlgCfg(flags))
   

    from MuonPatternRecognitionTest.PatternTestConfig import MuonR4PatternRecoChainCfg, MuonR4SegmentRecoChainCfg
    if args.houghR4:
        cfg.merge(MuonR4PatternRecoChainCfg(flags))

    ### What happens if you parse the R4 patterns to the legacy chain?
    cfg.merge(MuonR4SegmentRecoChainCfg(flags))

    from MuonPatternRecognitionTest.PatternTestConfig import TrackTruthMatchCfg
    cfg.merge(TrackTruthMatchCfg(flags, setupHoughR4 = args.houghR4))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonRecoChainTesterCfg
    cfg.merge(MuonRecoChainTesterCfg(flags,
                                    SegmentFromR4HoughKey = "MuonSegmentsFromHoughR4" if args.houghR4 else "" ))
    if args.runVtune: 
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))
    
    if args.monitorPlots:
        from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="EtaHoughPlotValid",
                                                                                                AllCanvasName="AllEtaHoughiDiPuffPlots", doPhiBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonPhiHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="PhiHoughPlotValid",
                                                                                                AllCanvasName="AllPhiHoughiDiPuffPlots",doEtaBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="SegmentPlotValid",
                                                                                                AllCanvasName="AllSegmentFitPlots", doPhiBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = True, saveSummaryPDF= False))
    executeTest(cfg)
    
