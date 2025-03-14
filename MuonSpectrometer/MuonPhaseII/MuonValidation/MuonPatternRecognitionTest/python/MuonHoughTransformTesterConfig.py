# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.add_argument("--writeSpacePoints", help="If set to true, the spacepoints in the bucket are saved to disk",
                                              default=False, action='store_true')
    parser.set_defaults(nEvents = -1)
   
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)

   
    parser.set_defaults(outRootFile="HoughTransformTester.root")
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2024-03")
    parser.set_defaults(inputFile=[
                                    #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"
                                    ])
    parser.set_defaults(eventPrintoutLevel = 50)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags.Muon.doFastMMDigitization = True
    flags, cfg = setupGeoR4TestCfg(args,flags)
  
    
    # cfg.getService("MessageSvc").setVerbose = ["MuonSegmentFittingAlg"]
    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg

    cfg.merge(MuonPatternRecognitionCfg(flags))
    if flags.Input.isMC:
        ## Keep them to manually exchange the map
        # "MDTTwinMapping_compactFormat_allBO", "MDTTwinMapping_compactFormat_fullSpectrometer",  
        # "MDTTwinMapping_compactFormat_Run123",  
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        cfg.merge(addOverride(flags, "/MDT/TWINMAPPING", "MDTTwinMapping_compactFormat_Run123"))


    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        cfg.merge(MuonSegmentFittingAlgCfg(flags, name = "MuonSegmentFittingAlg", ReadKey = "MuonHoughNswSegmentSeeds" ,MuonSegmentContainer = "R4NswSegments"))
        cfg.merge(MuonHoughTransformTesterCfg(flags, SegmentSeedKey = "MuonHoughNswSegmentSeeds", SegmentKey = "R4NswSegments" ))  
    else:
        cfg.merge(MuonSegmentFittingAlgCfg(flags))
        cfg.merge(MuonHoughTransformTesterCfg(flags, writeSpacePoints = args.writeSpacePoints))  

    if not args.noMonitorPlots and (flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC):
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
                                                                                                AllCanvasName="AllSegmentFitPlots",                                   displayTruthOnly = True,
                                                                                                saveSinglePDFs = True, saveSummaryPDF= False))
    if not args.noMonitorPlots and (flags.Detector.GeometryMM or flags.Detector.GeometrysTGC):
        cfg.getEventAlgo("MuonNswEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="NswEtaHoughPlotValid",
                                                                                                AllCanvasName="AllNswEtaHoughiDiPuffPlots",
                                                                                                saveSinglePDFs = True, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonNswPhiSeedFinderAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="NswPhiHoughPlotValid",
                                                                                                AllCanvasName="AllNswPhiHoughiDiPuffPlots",
                                                                                                saveSinglePDFs = True, saveSummaryPDF= False))

    executeTest(cfg)
    
