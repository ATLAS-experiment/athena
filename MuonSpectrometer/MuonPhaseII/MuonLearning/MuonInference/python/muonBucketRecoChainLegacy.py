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
    from MuonInference.InferenceConfig import (
        DEFAULT_BUCKET_MODEL_PATH,
        DEFAULT_BUCKET_SCORE_THRESHOLD,
        DEFAULT_BUCKET_SINGLE_OUTPUT_MODE,
    )
    parser.add_argument("--bucket-model-path", dest="bucket_model_path", default=DEFAULT_BUCKET_MODEL_PATH)
    parser.add_argument("--score-threshold", type=float, default=DEFAULT_BUCKET_SCORE_THRESHOLD, dest="score_threshold")
    parser.add_argument("--output-name", default="logits", dest="output_name")
    score_mode = parser.add_mutually_exclusive_group()
    score_mode.add_argument("--single-output-mode", choices=("logit", "prob"), default=DEFAULT_BUCKET_SINGLE_OUTPUT_MODE, dest="single_output_mode",
        help="Scalar ONNX-output interpretation. 'logit' applies sigmoid before thresholding.")
    score_mode.add_argument("--is-logit", action="store_const", const="logit", dest="single_output_mode", help="alias for --single-output-mode logit.",)
    score_mode.add_argument("--is-prob", action="store_const", const="prob", dest="single_output_mode", help="alias for --single-output-mode prob.",)
    parser.add_argument("--athenaDebug", action="store_true", default=False, help="Enable DEBUG verbosity for bucket inference components")
    parser.add_argument("--athenaVerbose", action="store_true", default=False, help="Enable VERBOSE verbosity for bucket inference components")
    parser.add_argument("--bucket-filter-summary", action="store_true", default=False, help="Print compact per-event input/selected/rejected bucket counts")
    parser.add_argument("--bucket-print-labels", action="store_true", default=False, help="Also print truth/segment expected-signal summary for each event")
    parser.add_argument("--bucket-label-print-first-n-buckets", type=int, default=0, dest="bucket_label_print_first_n_buckets",
                        help="Print detailed label/decision lines for the first N buckets per event")

    args = parser.parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"
    if args.athenaDebug or args.athenaVerbose:
        flags.Common.MsgSuppression = False

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
        
        output_level = 1 if args.athenaVerbose else (2 if args.athenaDebug else 3)
        bucket_tool_kwargs = dict(
            ModelPath=args.bucket_model_path,
            ScoreThreshold=args.score_threshold,
            OutputName=args.output_name,
            SingleOutputMode=args.single_output_mode,
            OutputLevel=output_level,
            PrintFilterSummary=args.bucket_filter_summary,
            PrintLabels=args.bucket_print_labels,
            LabelPrintFirstNBuckets=args.bucket_label_print_first_n_buckets,
        )
        if args.bucket_print_labels:
            if flags.Input.isMC:
                from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
                bucket_tool_kwargs["LabelVisualizationTool"] = cfg.popToolsAndMerge(
                    PatternVisualizationToolCfg(flags, CanvasLimits=0))
            else:
                # LegacyMuonRecoChainCfg has already built the segment container.
                bucket_tool_kwargs["LabelSegmentKey"] = "MuonSegmentsFromR4"
                
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(flags, **bucket_tool_kwargs)
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                "BucketFilterAlg",
                InferenceTools=[bucketTool],
            )
        )

        ## TODO: Train a separate NSW bucket model
        # NSW space points are produced in a separate container 
        nsw_bucket_tool_kwargs = dict(bucket_tool_kwargs)
        nsw_bucket_tool_kwargs.update(
            ReadSpacePoints="NswSpacePoints",
            WriteSpacePointKey="NswFilteredMlBuckets",
        )
        nswBucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags, "NswGraphBucketFilterTool", **nsw_bucket_tool_kwargs
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                "NswBucketFilterAlg",
                InferenceTools=[nswBucketTool],
            )
        )

    ### Setup the new chain
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))    
    
    if not args.skip_onnx:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"
        cfg.getEventAlgo("MuonNswEtaHoughTransformAlg").SpacePointContainer = "NswFilteredMlBuckets"

    from MuonPatternRecognitionTest.PatternTestConfig import MuonR4PatternRecoChainCfg, MuonR4SegmentRecoChainCfg
    if args.houghR4:
        cfg.merge(MuonR4PatternRecoChainCfg(flags))

    ### What happens if you parse the R4 patterns to the legacy chain
    cfg.merge(MuonR4SegmentRecoChainCfg(flags))

    from MuonPatternRecognitionTest.PatternTestConfig import TrackTruthMatchCfg
    cfg.merge(TrackTruthMatchCfg(flags, setupHoughR4 = args.houghR4))


    from MuonPatternRecognitionTest.PatternTestConfig import MuonRecoChainTesterCfg
    cfg.merge(MuonRecoChainTesterCfg(flags,
                                    SegmentFromR4HoughKey = "MuonSegmentsFromHoughR4" if args.houghR4 else "" ))

    if args.runVtune: 
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg", "BucketFilterAlg"]))
    
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