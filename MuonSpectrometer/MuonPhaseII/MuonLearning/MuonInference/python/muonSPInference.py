# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="InferenceHoughTest.root")
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    
    parser.add_argument("--noPerfMon", help="If set to true, full perfmonMT is enabled",
                        default=False, action='store_true')
    parser.add_argument("--use-cpu", action="store_true", default=False,
                       help="Use CPU for ONNX inference")
    
    args = parser.parse_args()
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    use_gpu_requested = not args.use_cpu
    # Runtime check for GPU availability. Prefer ONNXRuntime provider list,
    # fall back to PyTorch if ONNX runtime isn't available.
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
    from MuonInference.InferenceConfig import GraphSPFilterToolCfg, GraphInferenceAlgCfg
    cfg.merge(GraphInferenceAlgCfg(flags,InferenceTools = [cfg.popToolsAndMerge(GraphSPFilterToolCfg(flags))]))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))
    cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlSpacePoints"
    
    cfg.merge(setupHistSvcCfg(  flags,outFile=args.outRootFile,
                                outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg
    
    cfg.merge(MuonHoughTransformTesterCfg(  flags,
                                            VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))
    
    executeTest(cfg)
    

