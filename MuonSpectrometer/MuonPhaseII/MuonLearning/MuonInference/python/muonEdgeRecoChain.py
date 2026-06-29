#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# End-to-end test: bucket filter -> segment edge inference -> ML-assisted track seeding.

def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON = "perfmonmt_MuonR4Reco.json"
    flags.Trigger.Muon.useNewRegionSelector = False

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
        flags.AthOnnx.ExecutionProvider = (
            OnnxRuntimeType.CPU if args.use_cpu else OnnxRuntimeType.CUDA
        )
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU

    flags, cfg = setupGeoR4TestCfg(args, flags)

    if not args.skipTrackTester:
        cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile,
                                  outStream="MuonTrackTester"))

    output_level = 1 if args.athenaDebug else 3

    if run_bucket_filter:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                ModelPath=args.bucket_model_path,
                ScoreThreshold=args.score_threshold,
                OutputName=args.output_name,
                SingleOutputMode=args.single_output_mode,
                OutputLevel=output_level,
            )
        )
        cfg.merge(GraphInferenceAlgCfg(flags, InferenceTools=[bucketTool]))

    from MuonConfig.ReconstructionConfigR4 import MuonReconstructionConfig
    cfg.merge(MuonReconstructionConfig(flags))
    if run_bucket_filter:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"

    if run_edge_classifier:
        from MuonInference.InferenceConfig import SegmentEdgeInferenceAlgCfg
        edge_classifier_kwargs = {
            "ModelPath": args.edgeModel,
            "ReadSpacePoints": (
                "FilteredMlBuckets" if run_bucket_filter else "MuonSpacePoints"
            ),
        }
        cfg.merge(SegmentEdgeInferenceAlgCfg(
            flags,
            EdgeClassifierTool=edge_classifier_kwargs,
            EdgeThreshold=args.edgeThreshold,
            OverlapThreshold=args.overlapThreshold,
            UseRecoveryComponents=args.useRecoveryComponents,
            OutputLevel=output_level,
        ))

    if run_ml_seeder and not run_edge_classifier:
        print("WARNING: ML seeder enabled while edge classifier is disabled."
              " The decoration 'trackCandidateIds' may be missing.")

    ms_track_finder = cfg.getEventAlgo("MSTrackFinderAlg")
    ms_track_finder.UseMlSeeder = run_ml_seeder
    ms_track_finder.MlCandidateDecoration = "trackCandidateIds"
    ms_track_finder.MlFallbackToBaselineIfUndecorated = True
    ms_track_finder.MlFallbackToBaselineIfNoCandidates = True

    if not args.skipTrackTester:
        from MuonTrackFindingTest.MsTrackFindingTester import MsTrackTesterCfg
        cfg.merge(MsTrackTesterCfg(flags, scheduleLegacy=False, outFile=args.outRootFile))

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
    from MuonInference.InferenceConfig import (
        DEFAULT_BUCKET_MODEL_PATH,
        DEFAULT_BUCKET_SCORE_THRESHOLD,
        DEFAULT_BUCKET_SINGLE_OUTPUT_MODE,
    )
    parser.add_argument("--bucketModel", "--bucket-model-path", dest="bucket_model_path", default=DEFAULT_BUCKET_MODEL_PATH)
    parser.add_argument("--bucketThreshold", "--score-threshold", dest="score_threshold", type=float, default=DEFAULT_BUCKET_SCORE_THRESHOLD)
    parser.add_argument("--output-name", default="logits", dest="output_name",
                        help="Bucket filter ONNX output tensor name")
    score_mode = parser.add_mutually_exclusive_group()
    score_mode.add_argument("--single-output-mode", choices=("logit", "prob"), default=DEFAULT_BUCKET_SINGLE_OUTPUT_MODE, dest="single_output_mode",
                            help="Scalar ONNX-output interpretation. 'logit' applies sigmoid before thresholding.")
    score_mode.add_argument("--is-logit", action="store_const", const="logit", dest="single_output_mode",
                            help="Alias for --single-output-mode logit.")
    score_mode.add_argument("--is-prob", action="store_const", const="prob", dest="single_output_mode",
                            help="Alias for --single-output-mode prob.")    
    parser.add_argument("--edgeModel")
    parser.add_argument("--athenaDebug", action="store_true",
                        help="Enable Athena DEBUG verbosity for inference and seeding components")
    parser.add_argument("--noPerfMon", default=False, action="store_true",
                        help="Disable performance monitoring")
    parser.add_argument("--edgeThreshold", type=float, default=0.01,
                        help="Loose threshold for recovery components")
    parser.add_argument("--overlapThreshold", type=float, default=0.20,
                        help="High-purity threshold for core components")
    parser.add_argument("--useRecoveryComponents", action="store_true", default=True,
                        help="Use loose recovery connected components")
    parser.add_argument("--enableRecoChainTester", action="store_true", default=False,
                        help="Enable MuonRecoChainTester (can crash for some custom chains)")
    parser.add_argument("--skipTrackTester", action="store_true", default=False,
                        help="Do not write the MsTrackValidTest validation tree")
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
    parser.add_argument("--use-cpu", action="store_true", default=False,
                        help="Force CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False,
                        help="Skip all ONNX inference stages (bucket filter + edge classifier)")

    args = parser.parse_args()
    
    if not args.skip_onnx and args.enableEdgeClassifier and not args.edgeModel:
        parser.error("--edgeModel is required when edge classifier is enabled")
    main(args)
