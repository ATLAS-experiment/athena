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
    filter_segment_container = (
        args.filterSegmentsWithoutMlConnections and run_edge_classifier
    )
    filtered_segment_key = "MuonSegmentsFromR4MlConnected"

    if args.skip_onnx and (args.enableBucketFilter or args.enableEdgeClassifier):
        print("INFO: --skip-onnx requested. Disabling bucket filter and edge classifier inference stages.")
    if args.skip_onnx and args.useMlSeeder:
        print("INFO: --skip-onnx requested. Switching to the standard seeder for the non-ONNX baseline.")
    if args.filterSegmentsWithoutMlConnections and not run_edge_classifier:
        print("WARNING: --filterSegmentsWithoutMlConnections requires the edge "
              "classifier and will be ignored.")

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
            # These cuts run before ONNX.  MaxEdgesPerSegment below acts only
            # after all model scores have already been computed.
            "MaxSegmentsPerBucket": args.maxSegmentsPerBucket,
            "MaxEdgesPerNodeBeforeInference": args.maxEdgesBeforeInference,
            "MaxEdgesPerTargetChamberBeforeInference": (
                args.maxEdgesPerTargetChamber
            ),
            "DropSameChamberEdgesBeforeInference": (
                not args.keepSameChamberEdgesBeforeInference
            ),
            "DropIsolatedNodesBeforeInference": (
                not args.keepIsolatedNodesBeforeInference
            ),
            "EnableTruthDiagnostics": args.truthDiagnostics,
        }
        if args.maxDeltaThetaDeg is not None:
            edge_classifier_kwargs["MaxDeltaThetaDeg"] = args.maxDeltaThetaDeg
        edge_inference_kwargs = {
            "EdgeClassifierTool": edge_classifier_kwargs,
            "PairGateDecoration": "MuonSegmentsFromR4.mlTrackComponent",
            "PairGateThreshold": args.edgeThreshold,
            "MaxEdgesPerNode": args.maxEdgesPerSegment,
            "UseDegreeCappedComponents": args.useDegreeCappedMlComponents,
            "RequireMutualTopKEdges": not args.allowOneSidedMlEdges,
            "RecoverOrphanNodes": not args.disableOrphanRecovery,
            "SeedAnchorsPerComponent": args.seedAnchorsPerComponent,
            "AnchorInnermostLayer": args.anchorInnermostLayer,
            "MinSegmentsPerComponent": args.minSegmentsPerComponent,
            "KeepBestSegmentPerChamber": not args.keepAllSegmentsPerChamber,
            "OutputLevel": output_level,
        }
        if filter_segment_container:
            edge_inference_kwargs["FilteredSegmentKey"] = filtered_segment_key
        if args.truthDiagnostics and not flags.Input.isMC:
            print("WARNING: --truthDiagnostics requested on non-MC input.")
        cfg.merge(SegmentEdgeInferenceAlgCfg(flags, **edge_inference_kwargs))

    if run_ml_seeder and not run_edge_classifier:
        print("WARNING: ML seeder enabled while edge classifier is disabled."
              " The decoration 'mlTrackComponent' may be missing.")

    ms_track_finder = cfg.getEventAlgo("MSTrackFinderAlg")
    ms_track_finder.OutputLevel = output_level
    if run_ml_seeder:
        from MuonTrackFindingAlgs.TrackFindingConfig import MsTrackSeedingToolCfg
        from AthenaConfiguration.ComponentFactory import CompFactory
        baseline_seeder = cfg.popToolsAndMerge(MsTrackSeedingToolCfg(flags))
        ml_seeder_segment_container = (
            filtered_segment_key if filter_segment_container else "MuonSegmentsFromR4"
        )
        ms_track_finder.SeedingTool = CompFactory.MuonR4.MlMsTrackSeeder(
            "MlMsTrackSeeder",
            BaselineSeeder=baseline_seeder,
            SegmentContainer=ml_seeder_segment_container,
            CandidateDecoration="mlTrackComponent",
            MinSegmentsPerCandidate=args.minSegmentsPerComponent,
        )
    elif filter_segment_container:
        ms_track_finder.SeedingTool.SegmentContainer = filtered_segment_key

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
    parser.add_argument("--maxDeltaThetaDeg", type=float, default=None,
                        help="Override the edge-building opening-angle gate (deg); 180 disables it")
    parser.add_argument("--athenaDebug", action="store_true",
                        help="Enable Athena DEBUG verbosity for inference and seeding components")
    parser.add_argument("--truthDiagnostics", action="store_true", default=False,
                        help="MC-only: DEBUG-log a truth-vs-background breakdown of the segment-edge selection.")
    parser.add_argument("--noPerfMon", default=False, action="store_true",
                        help="Disable performance monitoring")
    parser.add_argument("--edgeThreshold", type=float, default=0.975,
                        help="Minimum high-confidence edge probability used to form ML track components")
    parser.add_argument("--maxEdgesPerSegment", type=int, default=2,
                        help="Keep this many highest-score neighbours per segment in the ML path graph (default: 2)")
    parser.add_argument("--useDegreeCappedMlComponents", action="store_true", default=False,
                        help="Use a global greedy degree cap instead of mutual top-K path extraction")
    parser.add_argument("--allowOneSidedMlEdges", "--allowBranchingMlComponents",
                        dest="allowOneSidedMlEdges", action="store_true", default=False,
                        help="Keep an edge selected by only one endpoint; use only for validation/recovery")
    parser.add_argument("--disableOrphanRecovery", action="store_true", default=False,
                        help="Disable bounded one-sided recovery for nodes with no mutual top-K ML edge")
    parser.add_argument("--seedAnchorsPerComponent", type=int, default=0,
                        help="Launch this many ranked ML anchors per component; zero keeps every retained segment (default: 0)")
    parser.add_argument("--anchorInnermostLayer", action="store_true", default=False,
                        help="Restrict seed anchors to inner segment(s)")
    parser.add_argument("--minSegmentsPerComponent", type=int, default=2,
                        help="Require this many retained chambers in an ML component before seeding (default: 2)")
    parser.add_argument("--maxSegmentsPerBucket", type=int, default=0,
                        help="Keep at most this many best duplicate segments in each "
                             "(sector,chamber,eta) bucket before ONNX; 0 keeps all.")
    parser.add_argument("--maxEdgesBeforeInference", type=int, default=6,
                        help="Each node nominates this many geometrically best "
                             "undirected edges before ONNX; 0 keeps all")
    parser.add_argument("--maxEdgesPerTargetChamber", type=int, default=1,
                        help="Keep at most this many geometrical neighbours from a "
                             "single target chamber for each node before ONNX; 0 keeps all")
    parser.add_argument("--keepSameChamberEdgesBeforeInference",
                        action="store_true", default=False,
                        help="Keep same-chamber edges in the ONNX input graph. "
                             "Disabled by default because direct ML seeding keeps "
                             "only one segment per chamber.")
    parser.add_argument("--keepIsolatedNodesBeforeInference",
                        action="store_true", default=False,
                        help="Keep nodes with no retained pre-ONNX edge. Disabled by "
                             "default because isolated nodes cannot contribute to edge scores.")
    chamber_representatives = parser.add_mutually_exclusive_group()
    chamber_representatives.add_argument("--keepAllSegmentsPerChamber", dest="keepAllSegmentsPerChamber", 
                        action="store_true", default=True, help="Keep all ML component segments from a chamber")
    chamber_representatives.add_argument(
        "--keepBestSegmentPerChamber", dest="keepAllSegmentsPerChamber", action="store_false",
        help="Keep only the highest-ranked segment per chamber")
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
    parser.add_argument("--filterSegmentsWithoutMlConnections",
                        "--filter-segments-without-ml-connections",
                        action="store_true", default=False,
                        help="Pass MSTrackFinderAlg a VIEW containing only "
                             "segments incident to a selected ML edge")
    parser.add_argument("--useMlSeeder", dest="useMlSeeder", action="store_true", default=True,
                        help="Use new ML-assisted seeder (default)")
    parser.add_argument("--useStandardSeeder", dest="useMlSeeder", action="store_false",
                        help="Use the standard seeder")
    parser.add_argument("--use-cpu", action="store_true", default=False,
                        help="Force CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False,
                        help="Skip all ONNX inference stages (bucket filter + edge classifier)")

    args = parser.parse_args()
    
    if not args.skip_onnx and args.enableEdgeClassifier and not args.edgeModel:
        parser.error("--edgeModel is required when edge classifier is enabled")
    main(args)
