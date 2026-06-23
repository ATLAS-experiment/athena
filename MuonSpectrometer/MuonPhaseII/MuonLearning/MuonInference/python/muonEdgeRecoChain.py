#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# End-to-end test: bucket filter -> segment edge inference -> ML-assisted track seeding.

def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    run_bucket_filter = args.enableBucketFilter and not args.skip_onnx
    run_edge_classifier = args.enableEdgeClassifier and not args.skip_onnx
    run_ml_seeder = args.useMlSeeder and not args.skip_onnx

    if args.skip_onnx and (args.enableBucketFilter or args.enableEdgeClassifier):
        print("INFO: --skip-onnx requested. Disabling bucket filter and edge classifier inference stages.")
    if args.skip_onnx and args.useMlSeeder:
        print("INFO: --skip-onnx requested. Switching to legacy seeder for a non-ONNX baseline.")

    if args.athenaDebug:
        flags.Exec.DebugMessageComponents = [
            "GraphInferenceAlg",
            "GraphInferenceAlg.GraphBucketFilterTool",
            "GraphInferenceAlg.GraphBucketFilterTool.OnnxRuntimeSessionToolCPU",
            "GraphInferenceAlg.GraphBucketFilterTool.OnnxRuntimeSessionToolCUDA",
            "SegmentEdgeInferenceAlg",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCPU",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCUDA",
            "SegmentEdgeInferenceAlg.SegmentTrackCandidateBuilderTool",
            "MSTrackFinderAlg",
            "MSTrackFinderAlg.MlMsTrackSeeder",
        ]

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    if run_bucket_filter or run_edge_classifier:
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
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU

    flags, cfg = setupGeoR4TestCfg(args, flags)

    cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile,
                              outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    cfg.merge(MuonSpacePointFormationCfg(flags))

    output_level = 1 if args.athenaDebug else 3

    if run_bucket_filter:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = cfg.popToolsAndMerge(GraphBucketFilterToolCfg(flags,
                                                                    ModelPath=args.bucketModel,
                                                                    ScoreThreshold=args.bucketThreshold,
                                                                    OutputLevel=output_level))
        cfg.merge(GraphInferenceAlgCfg(flags, InferenceTools=[bucketTool]))

    from MuonConfig.ReconstructionConfigR4 import MuonReconstructionConfig
    cfg.merge(MuonReconstructionConfig(flags))
    if run_bucket_filter:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"

    if run_edge_classifier:
        from MuonInference.InferenceConfig import SegmentEdgeInferenceAlgCfg
        cfg.merge(SegmentEdgeInferenceAlgCfg(flags,
                                             EdgeModelPath=args.edgeModel,
                                             EdgeThreshold=args.edgeThreshold,
                                             OverlapThreshold=args.overlapThreshold,
                                             UseRecoveryComponents=args.useRecoveryComponents,
                                             OutputLevel=output_level))
        
    if run_ml_seeder and not run_edge_classifier:
        print("WARNING: ML seeder enabled while edge classifier is disabled."
              " The decoration 'trackCandidateIds' may be missing.")

    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    cfg.merge(MSTrackFinderAlgCfg(flags,
                                  UseMlSeeder=run_ml_seeder,
                                  MlCandidateDecoration="trackCandidateIds",
                                  TrackingGeometryTool=cfg.getPrimaryAndMerge(
                                      ActsTrackingGeometryToolCfg(flags)),
                                  MlFallbackToBaselineIfUndecorated=True,
                                  MlFallbackToBaselineIfNoCandidates=False))

    if args.enableRecoChainTester:
        from MuonTrackFindingAlgs.TrackFindingConfig import MuonActsToTrkConvCfg
        cfg.merge(MuonActsToTrkConvCfg(flags,
                                       ACTSTracksLocation="MsTracks",
                                       TracksLocation="MsTracksConv"))
        
        from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
        cfg.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,
                                                       name="MuonXAODParticleConvR4",
                                                       TrackContainerName="MsTracksConv",
                                                       xAODTrackParticlesFromTracksContainerName="MuonSpectrometerTrackParticlesR4"))

        if flags.Input.isMC:
            from MuonTruthAlgsR4.MuonTruthAlgsConfig import RecoSegmentTruthAssocCfg, TrackToTruthPartAssocCfg
            cfg.merge(RecoSegmentTruthAssocCfg(flags,
                                               name="MuonSegmentsFromR4TruthMatching",
                                               SegmentKey="MuonSegmentsFromR4"))
            cfg.merge(TrackToTruthPartAssocCfg(flags,
                                               name="TrackToTruthMuonSpectrometerTrackParticlesR4",
                                               TrackCollection="MuonSpectrometerTrackParticlesR4"))

        from MuonPatternRecognitionTest.PatternTestConfig import MuonRecoChainTesterCfg
        cfg.merge(MuonRecoChainTesterCfg(flags,
                                         LegacySegmentKey="MuonSegmentsFromR4",
                                         SegmentFromR4HoughKey="",
                                         R4SegmentKey="MuonSegmentsFromR4",
                                         LegacyTrackKey="MuonSpectrometerTrackParticlesR4",
                                         TrackKeyHoughR4="",
                                         TrackKeyR4="MuonSpectrometerTrackParticlesR4"))

    cfg.printConfig(withDetails=True, summariseProps=True)
    executeTest(cfg)

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents=-1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(outRootFile="EdgeRecoChain.root")
    parser.add_argument("--bucketModel")
    parser.add_argument("--bucketThreshold", "--score-threshold", dest="bucketThreshold", type=float, default=0.0,
                        help="Threshold on bucket filter score")
    parser.add_argument("--edgeModel")
    parser.add_argument("--athenaDebug", action="store_true",
                        help="Enable Athena DEBUG verbosity for inference and seeding components")
    parser.add_argument("--edgeThreshold", type=float, default=0.25,
                        help="Loose threshold for recovery components")
    parser.add_argument("--overlapThreshold", type=float, default=0.8,
                        help="High-purity threshold for core components")
    parser.add_argument("--useRecoveryComponents", action="store_true", default=True,
                        help="Use loose recovery connected components")
    parser.add_argument("--enableRecoChainTester", action="store_true", default=False,
                        help="Enable MuonRecoChainTester (can crash for some custom chains)")

    parser.add_argument("--enableBucketFilter", dest="enableBucketFilter", action="store_true", default=True,
                        help="Enable ML bucket filtering stage")
    parser.add_argument("--disableBucketFilter", dest="enableBucketFilter", action="store_false",
                        help="Disable ML bucket filtering stage")

    parser.add_argument("--enableEdgeClassifier", dest="enableEdgeClassifier", action="store_true", default=True,
                        help="Enable segment-edge classifier stage")
    parser.add_argument("--disableEdgeClassifier", dest="enableEdgeClassifier", action="store_false",
                        help="Disable segment-edge classifier stage")

    parser.add_argument("--useMlSeeder", dest="useMlSeeder", action="store_true", default=True,
                        help="Use new ML-assisted seeder (default)")
    parser.add_argument("--useOldSeeder", dest="useMlSeeder", action="store_false",
                        help="Use legacy seeder")
    parser.add_argument("--use-gpu", action="store_true", dest="use_gpu", default=True,
                        help="Use GPU for ONNX inference (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                        help="Force CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False,
                        help="Skip all ONNX inference stages (bucket filter + edge classifier)")

    args = parser.parse_args()
    if not args.skip_onnx and args.enableBucketFilter and not args.bucketModel:
        parser.error("--bucketModel is required when bucket filter is enabled")
    if not args.skip_onnx and args.enableEdgeClassifier and not args.edgeModel:
        parser.error("--edgeModel is required when edge classifier is enabled")
    main(args)
