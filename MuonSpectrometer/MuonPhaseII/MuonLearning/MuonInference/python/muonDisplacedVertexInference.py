# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Run/debug the MuonLearning DisplacedVertex event classifier.

Example:
python -m MuonInference.muonDisplacedVertexInference \
  --inputFile my.RDO.pool.root \
  --model-path MuonInference/models/edge_class_dv_mu200.onnx \
  --nEvents 10 \
  --debug \
  --print-every-event

The ONNX model is expected to embed feature normalization and consume raw
DisplacedVertex graph tensors: x, edge_index, edge_attr, n_muon_nodes.
"""

import os
import logging

logging.getLogger("onnxruntime").setLevel(logging.ERROR)
os.environ.setdefault("ORT_LOGGING_LEVEL", "3")

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest

    parser = SetupArgParser()
    parser.add_argument("--model-path", default="MuonInference/models/edge_class_dv_mu200.onnx")
    parser.add_argument("--segment-key", default="MuonSegmentsFromR4")
    parser.add_argument("--tower-container-key", default="CombinedTower")
    parser.add_argument("--min-tower-energy-mev", type=float, default=1000.0)
    parser.add_argument("--max-tower-segment-dr", type=float, default=0.4)
    parser.add_argument("--calo-r-max-mm", type=float, default=4250.0)
    parser.add_argument("--calo-z-max-mm", type=float, default=6500.0)
    parser.add_argument("--sector-modulo", type=int, default=16)
    parser.add_argument("--require-edges", action="store_true", default=False)
    parser.add_argument("--score-threshold", type=float, default=0.5)
    parser.add_argument("--threshold-mode", choices=["score", "raw"], default="score",
                        help="'score' compares the post-processed signal score/probability;" 
                             "'raw' compares the raw ONNX output/logit.")
    parser.add_argument("--single-output-mode", default="logit", choices=["auto", "logit", "prob"])
    parser.add_argument("--use-cpu", action="store_true", default=False)
    parser.add_argument("--no-reco", action="store_true", default=False,
                        help="Do not schedule MuonReconstructionConfig; use this when segments/towers already exist in the input")
    parser.add_argument("--use-filtered-buckets-for-dv-graph", action="store_true", default=False,
                        help="Feed FilteredMlBuckets to the DV graph builder")
    parser.add_argument("--doMLBucketFilter", dest="do_ml_bucket_filter", action="store_true", default=True,
                        help="Run the same bucket prefilter used when producing the DV training ROOT/H5 files")
    parser.add_argument("--no-ml-bucket-filter", dest="do_ml_bucket_filter", action="store_false",
                        help="Disable the bucket prefilter. This no longer matches the default DV training production.")
    parser.add_argument("--bucketModel", "--bucket-model", dest="bucket_model",
                        default="dev/MuonRecRTT/edgecnn_mu200.onnx",
                        help="Bucket prefilter ONNX model, matching the muonBucketDump --bucketModel option")
    parser.add_argument("--bucketThreshold", "--bucket-threshold", dest="bucket_threshold", type=float, default=0.160,
                        help="Bucket prefilter working point, matching the muonBucketDump --bucketThreshold option")
    parser.add_argument("--filtered-bucket-key", default="FilteredMlBuckets",
                        help="StoreGate key written by the bucket prefilter and read by the DV graph builder")
    parser.add_argument("--no-calo-towers", action="store_true", default=False,
                        help="Do not schedule CaloRecoCfg/CaloTowerMakerCfg. Use only when TowerContainerKey already exists in the input/event store.")
    parser.add_argument("--debug", action="store_true", default=False,
                        help="Set the DV tool and algorithm OutputLevel to DEBUG and dump the first graph entries")
    parser.add_argument("--debug-nodes", type=int, default=5)
    parser.add_argument("--debug-edges", type=int, default=10)
    parser.add_argument("--print-every-event", action="store_true", default=False)
    parser.add_argument("--decorate-event-info", action="store_true", default=False, 
                        help="Enable optional EventInfo DV decorations for validation/debug output")
    parser.set_defaults(nEvents=10)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R4)
    parser.set_defaults(defaultGeoFile="RUN4")

    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Trigger.Muon.useNewRegionSelector = False

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU if args.use_cpu else OnnxRuntimeType.CUDA

    flags, cfg = setupGeoR4TestCfg(args, flags)

    if not args.no_reco:
        from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
        cfg.merge(xAODUncalibMeasPrepCfg(flags))

        from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
        cfg.merge(MuonSpacePointFormationCfg(flags))

        from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
        cfg.merge(MuonPatternRecognitionCfg(flags))

    from MuonInference.InferenceConfig import DisplacedVertexInferenceAlgCfg

    output_level = 2 if args.debug else 3
    cfg.merge(DisplacedVertexInferenceAlgCfg(
        flags,
        ModelPath=args.model_path,
        SegmentKey=args.segment_key,
        TowerContainerKey=args.tower_container_key,
        MinTowerEnergyMeV=args.min_tower_energy_mev,
        MaxTowerSegmentDR=args.max_tower_segment_dr,
        CaloRMaxMm=args.calo_r_max_mm,
        CaloZMaxMm=args.calo_z_max_mm,
        SectorModulo=args.sector_modulo,
        DoMLBucketFilter=args.do_ml_bucket_filter,
        BucketModelPath=args.bucket_model,
        BucketThreshold=args.bucket_threshold,
        FilteredBucketKey=args.filtered_bucket_key,
        UseFilteredBucketsForDVGraph=args.use_filtered_buckets_for_dv_graph,
        RequireEdges=args.require_edges,
        DoCaloTowerBuild=not args.no_calo_towers,
        SingleOutputMode=args.single_output_mode,
        ScoreThreshold=args.score_threshold,
        ThresholdMode=args.threshold_mode,
        OutputLevel=output_level,
        DebugDumpFirstNNodes=args.debug_nodes if args.debug else 0,
        DebugDumpFirstNEdges=args.debug_edges if args.debug else 0,
        PrintEveryEvent=args.print_every_event,
        DecorateEventInfo=args.decorate_event_info,
    ))

    executeTest(cfg)