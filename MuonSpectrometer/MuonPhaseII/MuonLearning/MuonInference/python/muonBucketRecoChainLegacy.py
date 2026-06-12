# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaborationation

if __name__=="__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noMM=False)
    parser.set_defaults(noSTGC=False)
    parser.set_defaults(outRootFile="RecoChainTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    
    parser.add_argument("--monitorPlots", action='store_true', default=False, 
                        help="Setup monitoring plots of the pattern recognition")
    parser.add_argument("--runVtune", action='store_true', default = False,
                        help="runs VTune profiler service for the muon hough alg")
    parser.add_argument("--noPerfMon", default=False, action='store_true', help="If set to true, full perfmonMT is enabled")
    parser.add_argument("--houghR4", action="store_true", default = False, 
                        help="Schedules the R4 pattern -> legacy segment -> legacy track chain")
    parser.add_argument("--use-cpu", action="store_true", default=False, help="Use CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False, help="Skip ONNX inference step")
    parser.add_argument("--bucket-model-path", default="dev/MuonRecRTT/edgecnn_mu200.onnx",dest="bucket_model_path")
    parser.add_argument("--score-threshold", type=float, default=0.0, dest="score_threshold")
    parser.add_argument("--output-name", default="logits", dest="output_name")
    parser.add_argument("--is-logit", dest="is_logit", default=False, action="store_true", help="Interpret the single output directly and do not apply sigmoid")

    args = parser.parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType

    if args.use_cpu:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA

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

    if not args.skip_onnx:
        ### Setup the ONNX inference step
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                ModelPath=args.bucket_model_path,
                ScoreThreshold=args.score_threshold,
                OutputName=args.output_name,
                SingleOutputIsLogit=args.is_logit if hasattr(args, "is_logit") else False,
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                InferenceTools=[bucketTool],
            )
        )

    ### Setup the new chain
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))    
    
    if not args.skip_onnx:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"

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