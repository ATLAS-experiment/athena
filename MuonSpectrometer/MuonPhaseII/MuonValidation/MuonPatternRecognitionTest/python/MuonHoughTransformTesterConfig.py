# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.add_argument("--writeSpacePoints", help="If set to true, the spacepoints in the bucket are saved to disk",
                                              default=False, action='store_true')
    parser.add_argument("--noPerfMon", help="If set to true, full perfmonMT is enabled",
                                              default=False, action='store_true')
    parser.set_defaults(nEvents = -1)
 
    parser.set_defaults(outRootFile="HoughTransformTester.root")
    from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
    parser.set_defaults(inputFile = MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(eventPrintoutLevel = 50)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"

    flags.Muon.doFastMMDigitization = False
    flags.Muon.doFastsTGCDigitization = False

    flags, cfg = setupGeoR4TestCfg(args,flags)
  
    
    cfg.getService("MessageSvc").setVerbose = []
    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg

    cfg.merge(MuonPatternRecognitionCfg(flags))
    if flags.Input.isMC:
        ## Keep them to manually exchange the map
        # "MDTTwinMapping_compactFormat_allBO", "MDTTwinMapping_compactFormat_fullSpectrometer",  
        # "MDTTwinMapping_compactFormat_Run123",  
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        cfg.merge(addOverride(flags, "/MDT/TWINMAPPING", "MDTTwinMapping_compactFormat_Run123"))
        
    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                              VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))

    if not args.noMonitorPlots and (flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC):
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="EtaHoughPlotValid", doPhiBucketViews = False,
                                                                                                outSubDir="EtaHoughiDiPuffPlots", displayTruthOnly = True, 
                                                                                                saveSinglePDFs = True, saveSummaryPDF= True))

        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots", 
                                                                                                displayTruthOnly = True, saveSinglePDFs = True, saveSummaryPDF= True))
    if not args.noMonitorPlots and (flags.Detector.GeometryMM or flags.Detector.GeometrysTGC):
        cfg.getEventAlgo("MuonNswEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags,
                                                                                                CanvasPreFix="NswEtaHoughPlotValid",  outSubDir="EtaHoughiDiPuffPlots",
                                                                                                saveSinglePDFs = True, doPhiBucketViews = False, saveSummaryPDF= True))

        cfg.getEventAlgo("MuonNswSegmentFinderAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="NswPhiHoughPlotValid",
                                                                                                outSubDir="AllNswPhiHoughiDiPuffPlots",
                                                                                                saveSinglePDFs = True, doPhiBucketViews = False, saveSummaryPDF= True))
       

    executeTest(cfg)
    
