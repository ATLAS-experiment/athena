#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Run segment-edge ONNX inference with an optional JSONL parity dump.
"""

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
            "GraphInferenceAlg",
            "GraphBucketFilterTool",
        ]
        print("INFO: Exec.DebugMessageComponents configured:", flags.Exec.DebugMessageComponents)

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    if args.use_cpu:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA

    flags, cfg = setupGeoR4TestCfg(args, flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    if args.doMLBucketFilter:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucket_tool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                ModelPath=args.bucket_model_path,
                ScoreThreshold=args.score_threshold,
                OutputName=args.output_name,
                SingleOutputMode=args.single_output_mode,
            )
        )
        cfg.merge(GraphInferenceAlgCfg(flags, InferenceTools=[bucket_tool]))
        cfg.merge(MuonPatternRecognitionCfg(flags))
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"
    else:
        cfg.merge(MuonPatternRecognitionCfg(flags))

    output_level = 1 if args.athenaDebug else 3
    edge_space_point_key = "FilteredMlBuckets" if args.doMLBucketFilter else "MuonSpacePoints"

    edge_classifier_kwargs = {
        "ModelPath": args.edgeModel,
        "ReadSpacePoints": edge_space_point_key,
        "DebugDumpFile": args.segment_edge_debug_dump_file,
        "DebugDumpMaxEvents": args.segment_edge_debug_dump_max_events,
        "MaxDeltaThetaDeg": args.max_delta_theta_deg,
        "MaxDeltaSector": args.max_delta_sector,
        "SectorModulo": args.sector_modulo,
        "OutputLevel": output_level,
    }
    from MuonInference.InferenceConfig import SegmentEdgeInferenceAlgCfg
    cfg.merge(
        SegmentEdgeInferenceAlgCfg(
            flags,
            EdgeClassifierTool=edge_classifier_kwargs,
            EdgeThreshold=args.edge_threshold,
            OutputLevel=output_level,
        )
    )

    executeTest(cfg)

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents=-1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.add_argument("--edgeModel", "--edge-model", required=True, dest="edgeModel",
                        help="ONNX segment-edge classifier")
    parser.add_argument("--edge-threshold", type=float, default=0.00013,
                        help="Candidate-builder edge probability threshold")
    parser.add_argument("--max-delta-theta-deg", type=float, default=35.0,
                        help="Graph edge direction window in degrees")
    parser.add_argument("--max-delta-sector", type=int, default=1,
                        help="Graph edge sector window")
    parser.add_argument("--sector-modulo", type=int, default=16,
                        help="Sector wrap-around modulo")

    from MuonInference.InferenceConfig import (
        DEFAULT_BUCKET_MODEL_PATH,
        DEFAULT_BUCKET_SCORE_THRESHOLD,
        DEFAULT_BUCKET_SINGLE_OUTPUT_MODE,
    )
    parser.add_argument("--doMLBucketFilter", dest="doMLBucketFilter", action="store_true", default=True)
    parser.add_argument("--noMLBucketFilter", dest="doMLBucketFilter", action="store_false")
    parser.add_argument("--bucket-model-path", dest="bucket_model_path", default=DEFAULT_BUCKET_MODEL_PATH)
    parser.add_argument("--score-threshold", type=float, default=DEFAULT_BUCKET_SCORE_THRESHOLD)
    parser.add_argument("--output-name", default="logits", dest="output_name",
                        help="Bucket filter ONNX output tensor name")
    score_mode = parser.add_mutually_exclusive_group()
    score_mode.add_argument("--single-output-mode", choices=("logit", "prob"), default=DEFAULT_BUCKET_SINGLE_OUTPUT_MODE, dest="single_output_mode",
                            help="Scalar ONNX-output interpretation. 'logit' applies sigmoid before thresholding.")
    score_mode.add_argument("--is-logit", action="store_const", const="logit", dest="single_output_mode", 
                            help="Alias for --single-output-mode logit.")
    score_mode.add_argument("--is-prob", action="store_const", const="prob", dest="single_output_mode", 
                            help="Alias for --single-output-mode prob.")

    parser.add_argument("--segment-edge-debug-dump-file", default="",
                        help="Optional JSONL with exact x, edge_index, edge_attr, logits and probabilities")
    parser.add_argument("--segment-edge-debug-dump-max-events", type=int, default=0,
                        help="Maximum graph events written to the segment-edge JSONL dump; 0 means all")
    parser.add_argument("--athenaDebug", action="store_true",
                        help="Enable Athena DEBUG verbosity")
    parser.add_argument("--use-cpu", action="store_true", default=False, help="Force CPU for ONNX inference")
    
    args = parser.parse_args()
    main(args)
