#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Test only the segment-edge inference decoration.

def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    if args.athenaDebug:
        flags.Exec.DebugMessageComponents = [
            "SegmentEdgeInferenceAlg",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool",
            "SegmentEdgeInferenceAlg.SegmentTrackCandidateBuilderTool",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCPU",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCUDA",
        ]
        print("INFO: Exec.DebugMessageComponents configured:", flags.Exec.DebugMessageComponents)

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    use_gpu_requested = args.use_gpu if args.use_gpu is not None else True
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

    flags, cfg = setupGeoR4TestCfg(args, flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    output_level = 1 if args.athenaDebug else 3
    from MuonInference.InferenceConfig import SegmentEdgeInferenceAlgCfg
    cfg.merge(SegmentEdgeInferenceAlgCfg(flags, EdgeModelPath=args.edgeModel,
                                         OutputLevel=output_level))

    executeTest(cfg)

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents=-1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.add_argument("--edgeModel", required=True, help="ONNX segment edge classifier")
    parser.add_argument("--athenaDebug", action="store_true", help="Enable Athena DEBUG verbosity for MessageSvc and key algorithms")
    parser.add_argument("--use-gpu", action="store_true", dest="use_gpu", default=True,
                       help="Use GPU for ONNX inference (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                       help="Force CPU for ONNX inference")
    main(parser.parse_args())
