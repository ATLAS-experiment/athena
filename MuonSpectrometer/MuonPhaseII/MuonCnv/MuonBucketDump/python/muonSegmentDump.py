# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = False
    includeG4TrackTruth = bool(getattr(args, "includeG4TrackTruth", False))
    minG4TrackTruthSegments = int(getattr(args, "minG4TrackTruthSegments", 2))
    if minG4TrackTruthSegments < 1:
        raise ValueError("--minG4TrackTruthSegments must be >= 1")
    if includeG4TrackTruth:
        flags.Muon.includePileUpTruth = True

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType

    if getattr(args, "use_cpu", False):
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA

    flags, cfg = setupGeoR4TestCfg(args)

    cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile, outStream="MuonSegmentDump"))

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
        bucket_tool_kwargs: dict[str, object] = {"WriteSpacePointKey": "FilteredMlBuckets"}
        if getattr(args, "bucketModel", None) is not None:
            bucket_tool_kwargs["ModelPath"] = args.bucketModel
        if getattr(args, "bucketThreshold", None) is not None:
            bucket_tool_kwargs["ScoreThreshold"] = args.bucketThreshold
        bucket_tool_kwargs["OutputName"] = args.output_name
        bucket_tool_kwargs["SingleOutputIsLogit"] = args.is_logit if hasattr(args, "is_logit") else False
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
    dumper_kwargs = {
        "IncludeG4TrackTruth": includeG4TrackTruth,
        "MinG4TrackTruthSegments": minG4TrackTruthSegments,
    }

    if do_ml_bucket_filter:
        cfg.merge(MuonSegmentDumpCfg(flags, SpacePointKeys=["FilteredMlBuckets"],
                                     **dumper_kwargs))
    else:
        cfg.merge(MuonSegmentDumpCfg(flags, **dumper_kwargs))

    executeTest(cfg)


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents=-1)
    parser.set_defaults(outRootFile="MuonSegmentDump_R3SimHits.root")

    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.add_argument("--doMLBucketFilter", action="store_true", default=False,
                        help="Run ML bucket filtering and dump segments from filtered buckets.")
    parser.add_argument("--bucketModel", type=str, default=None,
                        help="Path to ONNX model used by the ML bucket filter.")
    parser.add_argument("--bucketThreshold", type=float, default=None,
                        help="Score threshold for single-output bucket filtering.")
    parser.add_argument("--output-name", type=str, default="logits", dest="output_name",
                        help="Name of the ONNX output node used by the ML bucket filter.")
    parser.add_argument("--is-logit", dest="is_logit", action="store_true", default=False,
                        help="Interpret the single output directly and do not apply sigmoid")
    parser.add_argument("--use-cpu", action="store_true", default=False,
                        help="Force CPU for ONNX inference")
    parser.add_argument("--includeG4TrackTruth", action="store_true", default=False,
                        help="Use sim-hit HepMC/G4 track identifiers for unmatched segment labels.")
    parser.add_argument("--minG4TrackTruthSegments", type=int, default=2,
                        help="Minimum number of reco segments that must share the same G4/HepMC id.")


    args = parser.parse_args()
    main(args)
