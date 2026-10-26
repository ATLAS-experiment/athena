#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# End-to-end test: segment edge inference -> ML segment filter -> ML-assisted track seeding.

import argparse


def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON = "perfmonmt_MuonR4Reco.json"
    flags.Trigger.Muon.useNewRegionSelector = False

    run_edge_classifier = not args.skip_onnx
    run_ml_seeder = args.useMlSeeder and run_edge_classifier
    filter_segment_container = args.filterSegmentsWithoutMlConnections and run_edge_classifier
    filtered_segment_key = "MsSegmentsMlConnected"

    if args.skip_onnx and args.useMlSeeder:
        print("INFO: --skip-onnx requested. Switching to the standard seeder for the non-ONNX baseline.")

    if args.athenaDebug:
        flags.Exec.DebugMessageComponents = [
            "SegmentEdgeInferenceAlg",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCPU",
            "SegmentEdgeInferenceAlg.SegmentEdgeClassifierTool.OnnxRuntimeSessionToolCUDA",
            "MSTrackFinderAlg.MlMsTrackSeeder",
        ]

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA if run_edge_classifier else OnnxRuntimeType.CPU

    flags, cfg = setupGeoR4TestCfg(args, flags)

    if not args.skipTrackTester:
        cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile,
                                  outStream="MuonTrackTester"))

    output_level = 1 if args.athenaDebug else 3

    from MuonConfig.ReconstructionConfigR4 import MuonReconstructionConfig
    cfg.merge(MuonReconstructionConfig(flags))

    if run_edge_classifier:
        if args.truthDiagnostics and not flags.Input.isMC:
            print("WARNING: --truthDiagnostics requested on non-MC input.")
        from MuonInference.InferenceConfig import SegmentEdgeInferenceAlgCfg
        edge_inference_kwargs = {
            "EdgeClassifierTool": {
                "ModelPath": args.edgeModel,
                "ReadSpacePoints": "MuonSpacePoints",
                # These cuts run before ONNX
                "MaxSegmentsPerBucket": args.maxSegmentsPerBucket,
                "MaxEdgesPerNodeBeforeInference": 6,
                "MaxEdgesPerTargetChamberBeforeInference": args.maxEdgesPerTargetChamber,
                "EnableTruthDiagnostics": args.truthDiagnostics,
            },
            "PairGateDecoration": "MsSegments.mlTrackComponent",
            "PairGateThreshold": args.edgeThreshold,
            "MaxEdgesPerNodePerLayer": args.maxEdgesPerLayer,
            # One anchor per component: the innermost segment with the strongest cross-layer edge
            "SeedAnchorsPerComponent": 1,
            "AnchorInnermostLayer": True,
            "KeepBestSegmentPerChamber": not args.keepAllSegmentsPerChamber,
            "OutputLevel": output_level,
        }
        if filter_segment_container:
            edge_inference_kwargs["FilteredSegmentKey"] = filtered_segment_key
        cfg.merge(SegmentEdgeInferenceAlgCfg(flags, **edge_inference_kwargs))

    ms_track_finder = cfg.getEventAlgo("MSTrackFinderAlg")
    ms_track_finder.OutputLevel = 3
    if run_ml_seeder:
        from MuonTrackFindingAlgs.TrackFindingConfig import MsTrackSeedingToolCfg, SegmentSelectorCfg
        from AthenaConfiguration.ComponentFactory import CompFactory
        baseline_seeder = cfg.popToolsAndMerge(MsTrackSeedingToolCfg(flags))
        ms_track_finder.SeedingTool = CompFactory.MuonR4.MlMsTrackSeeder(
            "MlMsTrackSeeder",
            BaselineSeeder=baseline_seeder,
            SegmentContainer="MsSegments",
            CandidateDecoration="mlTrackComponent",
            SegmentSelectionTool=cfg.popToolsAndMerge(SegmentSelectorCfg(flags)),
            TwoSegmentMinComponentScore=args.mlTwoSegMinScore,
            ExtraClusterSeeds=args.mlExtraClusterSeeds,
            ExtraClusterMinScore=args.mlExtraClusterMinScore,
            InnerPointingMaxDeg=args.mlInnerPointingMaxDeg,
            InnerRescueWindow=args.mlInnerRescueWindow,
            InnerRescueMaxDeg=args.mlInnerRescueMaxDeg,
            VariantSeeds=args.mlVariantSeeds,
            VariantMinLayers=args.mlVariantMinLayers,
            VariantMaxPointingDeg=args.mlVariantMaxPointingDeg,
            VariantMinScore=args.mlVariantMinScore,
            EnableTruthDiagnostics=args.truthDiagnostics and flags.Input.isMC,
            OutputLevel=2 if args.athenaDebug else 3,
        )
    elif filter_segment_container:
        ms_track_finder.SeedingTool.SegmentContainer = filtered_segment_key

    if not args.skipTrackTester:
        from MuonTrackFindingTest.MsTrackFindingTester import MsTrackTesterCfg
        cfg.merge(MsTrackTesterCfg(flags, scheduleLegacy=False, outFile=args.outRootFile))

    cfg.printConfig(withDetails=True, summariseProps=True)
    executeTest(cfg)


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents=-1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(outRootFile="EdgeRecoChain.root")
    parser.add_argument("--edgeModel", help="Segment edge classifier ONNX model")
    parser.add_argument("--athenaDebug", action="store_true",
                        help="Enable Athena DEBUG verbosity for inference and seeding components")
    parser.add_argument("--truthDiagnostics", action="store_true", default=False,
                        help="MC-only: DEBUG-log the truth diagnostics of the edge selection and of the seeding")
    parser.add_argument("--noPerfMon", default=False, action="store_true",
                        help="Disable performance monitoring")
    parser.add_argument("--edgeThreshold", type=float, default=0.01,
                        help="Minimum edge probability used to form ML track components (default: 0.01)")
    parser.add_argument("--filterSegmentsWithoutMlConnections", action=argparse.BooleanOptionalAction,
                        default=True,
                        help="Pass MSTrackFinderAlg a VIEW containing only segments incident to a "
                             "selected ML edge (default: on)")
    parser.add_argument("--useMlSeeder", dest="useMlSeeder", action="store_true", default=True,
                        help="Use the ML-assisted seeder (default)")
    parser.add_argument("--useStandardSeeder", dest="useMlSeeder", action="store_false",
                        help="Use the standard seeder")
    parser.add_argument("--mlTwoSegMinScore", type=float, default=0.3,
                        help="ML seeder: keep a 2-segment seed only if its component's strongest cross-layer "
                             "ML edge reaches this score; 0 disables (default: 0.3)")
    parser.add_argument("--mlExtraClusterSeeds", type=int, default=2,
                        help="ML seeder: after the first window seed of a component, build up to this many "
                             "further window seeds from the segments not yet covered; 0 disables (default: 2)")
    parser.add_argument("--mlExtraClusterMinScore", type=float, default=0.8,
                        help="ML seeder: an extra cluster seed needs a segment whose strongest selected "
                             "cross-layer ML edge reaches this score (default: 0.8)")
    parser.add_argument("--mlInnerPointingMaxDeg", type=float, default=2.,
                        help="ML seeder: drop endcap inner segments whose |theta(dir)-theta(pos)| exceeds this "
                             "value [deg] (not pointing to the IP) from the seeds; 0 disables (default: 2)")
    parser.add_argument("--mlInnerRescueWindow", type=float, default=0.,
                        help="ML seeder: fill a missing Inner layer with the closest IP-pointing inner segment "
                             "of any component within this distance [mm]; 0 disables (default: 0)")
    parser.add_argument("--mlInnerRescueMaxDeg", type=float, default=1.,
                        help="ML seeder: pointing cut [deg] of the segment used by --mlInnerRescueWindow (default: 1)")
    parser.add_argument("--mlVariantSeeds", action=argparse.BooleanOptionalAction, default=True,
                        help="ML seeder: emit a second, different seed (runner-up segments) for seeds passing "
                             "the --mlVariant* gates, as a fallback if the first fit fails (default: on)")
    parser.add_argument("--mlVariantMinLayers", type=int, default=3,
                        help="ML seeder: variant seeds only for seeds with at least this many station layers (default: 3)")
    parser.add_argument("--mlVariantMaxPointingDeg", type=float, default=1.,
                        help="ML seeder: variant seeds only for endcap seeds whose inner segment points to the IP "
                             "within this value [deg] (default: 1)")
    parser.add_argument("--mlVariantMinScore", type=float, default=0.8,
                        help="ML seeder: variant seeds only for components whose strongest cross-layer ML edge "
                             "reaches this score; 0 disables (default: 0.8)")
    parser.add_argument("--maxEdgesPerLayer", type=int, default=1,
                        help="After ONNX, each segment nominates its best N edges towards each layer instead "
                             "of its best 2 edges overall (mutual selection unchanged); 0 keeps the global "
                             "top-2 (default: 1)")
    parser.add_argument("--maxEdgesPerTargetChamber", type=int, default=1,
                        help="Before ONNX, each segment keeps at most this many geometrical neighbours "
                             "in one target chamber; 0 keeps all (default: 1)")
    parser.add_argument("--maxSegmentsPerBucket", type=int, default=0,
                        help="Keep at most this many best duplicate segments in each "
                             "(sector,chamber,eta) bucket before ONNX; 0 keeps all (default: 0)")
    parser.add_argument("--keepAllSegmentsPerChamber", action="store_true", default=True,
                        help="Keep all ML component segments from a chamber (default)")
    parser.add_argument("--disableBucketFilter", action="store_true", default=True,
                        help="No effect: the bucket filter is not part of this chain (kept so that "
                             "existing commands still run)")
    parser.add_argument("--skipTrackTester", action="store_true", default=False,
                        help="Do not write the MsTrackValidTest validation tree")
    parser.add_argument("--skip-onnx", action="store_true", default=False,
                        help="Skip the edge classifier: non-ML baseline with the standard seeder")

    args = parser.parse_args()

    if not args.skip_onnx and not args.edgeModel:
        parser.error("--edgeModel is required unless --skip-onnx is given")
    main(args)
