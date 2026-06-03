# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = False

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    use_gpu_requested = getattr(args, "use_gpu", True)
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

    flags, cfg = setupGeoR4TestCfg(args)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonSegmentDump"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg

    do_ml_bucket_filter = bool(getattr(args, "doMLBucketFilter", False) or
                               getattr(args, "bucketModel", None) is not None or
                               getattr(args, "bucketThreshold", None) is not None)
    if do_ml_bucket_filter:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucket_tool_kwargs = {"WriteSpacePointKey": "FilteredMlBuckets"}
        if getattr(args, "bucketModel", None) is not None:
            bucket_tool_kwargs["ModelPath"] = args.bucketModel
        if getattr(args, "bucketThreshold", None) is not None:
            bucket_tool_kwargs["ScoreThreshold"] = args.bucketThreshold
        bucket_tool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                **bucket_tool_kwargs,
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                InferenceTools=[bucket_tool],
            )
        )
        # Re-run pattern recognition on filtered buckets so dumped segments
        # correspond to the same filtered container.
        cfg.merge(MuonPatternRecognitionCfg(flags))
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"
    else:
        cfg.merge(MuonPatternRecognitionCfg(flags))

    # Truth information if MC
    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
        cfg.merge(MuonTruthAlgsCfg(flags))

    from MuonBucketDump.MuonBucketDumpConfig import MuonSegmentDumpCfg
    if do_ml_bucket_filter:
        cfg.merge(MuonSegmentDumpCfg(flags, SpacePointKeys=["FilteredMlBuckets"]))
    else:
        cfg.merge(MuonSegmentDumpCfg(flags))

    executeTest(cfg)

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonSegmentDump_R3SimHits.root")

    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.add_argument("--doMLBucketFilter", action="store_true", default=False,
                        help="Run ML bucket filtering and dump segments from filtered buckets.")
    parser.add_argument("--bucketModel", type=str, default=None,
                        help="Path to ONNX model used by the ML bucket filter.")
    parser.add_argument("--bucketThreshold", type=float, default=None,
                        help="Score threshold for single-output bucket filtering.")
    parser.add_argument("--use-gpu", action="store_true", default=True,
                        help="Use GPU for ONNX inference when available (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                        help="Force CPU for ONNX inference")

    args = parser.parse_args()
    main(args)

    