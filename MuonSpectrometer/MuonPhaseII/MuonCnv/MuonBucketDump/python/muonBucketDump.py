# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True

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
                                    outStream="MuonBucketDump"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    if getattr(args, "doMLBucketFilter", False):
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bias = getattr(args, "mlBucketBias", 1.0)
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags, 
                BiasClass0=bias, 
                WriteSpacePointKey="FilteredMlBuckets",
                ModelPath="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/edgecnn_multi_bucket_sparse_meta.onnx"
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags, 
                InferenceTools=[bucketTool],
            )
        )

    from MuonBucketDump.MuonBucketDumpConfig import MuonBucketDumpCfg
    from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
    cfg.merge(MuonBucketDumpCfg(flags,
                                DoCaloDump=getattr(args, "doCaloDump", False),
                                DoMLBucketScore=getattr(args, "doMLBucketScore", False),
                                DoMLBucketFilter=getattr(args, "doMLBucketFilter", False),
                                MLBucketBias=getattr(args, "mlBucketBias", 1.0),
                                VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))
    if args.doTruthMuonVertexDump:
        from MuonBucketDump.MuonBucketDumpConfig import TruthMuonVertexDumpCfg
        cfg.merge(TruthMuonVertexDumpCfg(flags))

    executeTest(cfg)

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonBucketDump_R3SimHits.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.add_argument("--doCaloDump", action="store_true", default=False, 
                        help="Run calorimeter reconstruction and dump cell energy/position.")

    parser.add_argument("--doMLBucketScore", action="store_true", default=False,
                        help="Run ML inference and dump bucket filter scores.")

    parser.add_argument("--doMLBucketFilter", action="store_true", default=False,
                        help="Run ML bucket filtering and write filtered buckets to 'FilteredMlBuckets' container.")

    parser.add_argument("--mlBucketBias", type=float, default=1.0,
                        help="Bias value for class 0 in ML bucket classification (default: 1.0). Higher values make class 0 less likely.")

    parser.add_argument("--doTruthMuonVertexDump", action="store_true", help="Run the TruthMuonVertexDumperAlg to dump truth muon vertex information", default=False)
    
    parser.add_argument("--use-gpu", action="store_true", default=True, 
                       help="Use GPU for ONNX inference (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                       help="Use CPU for ONNX inference")
    
    args = parser.parse_args()
    main(args)

    