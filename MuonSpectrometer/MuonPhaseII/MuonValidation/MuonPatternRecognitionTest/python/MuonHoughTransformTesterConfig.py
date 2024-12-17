# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.set_defaults(nEvents = -1)
    #parser.set_defaults(noMM=True)
    #parser.set_defaults(noSTGC=True)
    parser.set_defaults(outRootFile="HoughTransformTester.root")
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2023-02")
    parser.set_defaults(inputFile=[
                                    #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R3SimHits.pool.root"
                                    ])
    parser.set_defaults(eventPrintoutLevel = 500)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags.Muon.doFastMMDigitization = False
    flags, cfg = setupGeoR4TestCfg(args,flags)
    

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
    cfg.merge(MuonSegmentFittingAlgCfg(flags))
    cfg.merge(MuonHoughTransformTesterCfg(flags))    

    if flags.Input.isMC:
        ## Keep them to manually exchange the map
        # "MDTTwinMapping_compactFormat_allBO", "MDTTwinMapping_compactFormat_fullSpectrometer",  
        # "MDTTwinMapping_compactFormat_Run123",  
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        cfg.merge(addOverride(flags, "/MDT/TWINMAPPING", "MDTTwinMapping_compactFormat_Run123"))
   

    
    if not args.noMonitorPlots:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="EtaHoughPlotValid",
                                                                                                AllCanvasName="AllEtaHoughiDiPuffPlots",                                         displayTruthOnly = True,
                                                                                                saveSinglePDFs = True, saveSummaryPDF= True))
        cfg.getEventAlgo("MuonPhiHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="PhiHoughPlotValid",
                                                                                                AllCanvasName="AllPhiHoughiDiPuffPlots",                                   displayTruthOnly = True,
                                                                                                saveSinglePDFs = False, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="SegmentPlotValid",
                                                                                                AllCanvasName="AllSegmentFitPlots",                                   displayTruthOnly = True,
                                                                                                saveSinglePDFs = True, saveSummaryPDF= False))

    executeTest(cfg)
    
