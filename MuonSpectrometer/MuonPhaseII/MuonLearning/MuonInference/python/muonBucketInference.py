# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import logging

# Suppress ONNX Runtime warnings at Python logging level before Athena initialization
logging.getLogger("onnxruntime").setLevel(logging.ERROR)

# Set environment variable for ONNX Runtime before imports (attempt early suppression)
os.environ["ORT_LOGGING_LEVEL"] = "3"  # 3 = ERROR

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="InferenceHoughTest.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R4)
    parser.set_defaults(defaultGeoFile="RUN4")
    
    parser.add_argument("--doPerfMon", help="If set to true, full perfmonMT is enabled",
                        default=False, action='store_true')
    parser.add_argument("--use-cpu", action="store_true", default=False, help="Use CPU for ONNX inference")
    parser.add_argument("--athenaDebug", action="store_true", default=False,
                       help="Enable DEBUG verbosity for bucket inference components in MessageSvc")
    parser.add_argument("--athenaVerbose", action="store_true", default=False,
                       help="Enable VERBOSE verbosity for bucket inference components in MessageSvc")
    parser.add_argument("--bucket-model-path", dest="bucket_model_path", default="dev/MuonRecRTT/edgecnn_mu200.onnx",
                        help="Absolute path (or PathResolver key) for the bucket ONNX model")
    parser.add_argument("--score-threshold", type=float, default=0.2, dest="score_threshold",
                        help="Keep bucket if single-output score > threshold (default: 0.2)")
    parser.add_argument("--bucket-debug-dump-file", default="", dest="bucket_debug_dump_file",
                        help=("Optional JSONL output file with Athena-side bucket ONNX."))
    parser.add_argument("--bucket-debug-dump-max-events", type=int, default=0, dest="bucket_debug_dump_max_events",
                        help=("Maximum number of events to write to --bucket-debug-dump-file."))
    parser.add_argument("--bucket-print-labels", action="store_true", default=False, dest="bucket_print_labels",
                        help=("Print per-event good-bucket efficiency using the training label."))
    parser.add_argument("--bucket-label-print-first-n-buckets", type=int, default=20, dest="bucket_label_print_first_n_buckets",
                        help=("When --bucket-print-labels is enabled, print detailed label/decision."))
    parser.add_argument("--bucket-label-segment-key", default="MuonSegmentsFromR4", dest="bucket_label_segment_key",
                        help=("Optional xAOD::MuonSegmentContainer key used for the bucket_segments."))
    
    args = parser.parse_args()

    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = getattr(args, "doPerfMon", False)
    if args.athenaDebug or args.athenaVerbose:
        flags.Common.MsgSuppression = False

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    # Use command line argument if provided, otherwise default to True
    use_gpu_requested = not args.use_cpu
    gpu_available = False
    try:
        import onnxruntime as ort
        gpu_available = "CUDAExecutionProvider" in ort.get_available_providers()
    except Exception:
        try:
            import torch
            gpu_available = torch.cuda.is_available()
        except Exception:
            gpu_available = False
    if use_gpu_requested and gpu_available:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU

    flags, cfg = setupGeoR4TestCfg(args,flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))
    
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    

    sample_is_mc = bool(flags.Input.isMC)
    use_truth_labels = bool(args.bucket_print_labels and sample_is_mc)
    use_segment_labels = bool(args.bucket_print_labels and (not sample_is_mc))

    if use_segment_labels and not args.bucket_label_segment_key:
        raise RuntimeError(
            "--bucket-print-labels was requested for recorded data, but "
            "--bucket-label-segment-key is empty. Set it to MuonSegmentsFromR4 "
            "or disable label printing."
        )

    build_label_segments_first = bool(use_segment_labels)

    if build_label_segments_first:
        cfg.merge(MuonPatternRecognitionCfg(flags))
    
    from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
    output_level = 1 if args.athenaVerbose else (2 if args.athenaDebug else 3)

    bucket_tool_kwargs = dict(
        ModelPath=args.bucket_model_path,
        ScoreThreshold=args.score_threshold,
        DebugDumpFile=args.bucket_debug_dump_file,
        DebugDumpMaxEvents=args.bucket_debug_dump_max_events,
        PrintLabels=args.bucket_print_labels,
        LabelPrintFirstNBuckets=args.bucket_label_print_first_n_buckets,
        OutputLevel=output_level,
    )

    if args.bucket_print_labels:
        if use_truth_labels:
            from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
            bucket_tool_kwargs["LabelVisualizationTool"] = cfg.popToolsAndMerge(
                PatternVisualizationToolCfg(flags, CanvasLimits=0))
            print("Bucket label mode: MC input sample -> label = bucket with truth muons only")
        elif use_segment_labels:
            bucket_tool_kwargs["LabelSegmentKey"] = args.bucket_label_segment_key
            print("Bucket label mode: recorded-data input sample -> label = bucket_segments > 0 "
                  f"using {args.bucket_label_segment_key}")

    bucketTool = cfg.popToolsAndMerge(GraphBucketFilterToolCfg(flags, **bucket_tool_kwargs))
    cfg.merge(GraphInferenceAlgCfg(flags, InferenceTools=[bucketTool]))

    if not build_label_segments_first:
        cfg.merge(MuonPatternRecognitionCfg(flags))
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"
    
    cfg.merge(setupHistSvcCfg(  flags, outFile=args.outRootFile,
                                outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg
    
    cfg.merge(MuonHoughTransformTesterCfg(  flags,
                                            VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))

    executeTest(cfg)