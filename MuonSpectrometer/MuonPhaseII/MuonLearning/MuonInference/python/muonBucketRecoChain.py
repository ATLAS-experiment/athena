# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import logging

# Suppress ONNX Runtime warnings at Python logging level before Athena initialization
logging.getLogger("onnxruntime").setLevel(logging.ERROR)

# Set environment variable for ONNX Runtime before imports (attempt early suppression)
os.environ["ORT_LOGGING_LEVEL"] = "3"  # 3 = ERROR

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", default = False, action='store_true', help="If set to true, there're no monitoring plots")
    parser.add_argument("--writeSpacePoints", default=False, action='store_true', help="If set to true, the spacepoints in the bucket are saved to disk")
    parser.add_argument("--noPerfMon", default=False, action='store_true', help="If set to true, disable performance monitoring")
    parser.add_argument("--LegacyChain", default = False, action = 'store_true', help="If set to true, the legacy chain is not scheduled",)
    parser.add_argument("--use-cpu", default = False, action = 'store_true', help="Use CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False, help="Skip ONNX inference step")
    from MuonInference.InferenceConfig import (
        DEFAULT_BUCKET_MODEL_PATH,
        DEFAULT_BUCKET_SCORE_THRESHOLD,
        DEFAULT_BUCKET_SINGLE_OUTPUT_MODE,
    )
    parser.add_argument("--bucket-model-path", dest="bucket_model_path", default=DEFAULT_BUCKET_MODEL_PATH)
    parser.add_argument("--score-threshold", type=float, default=DEFAULT_BUCKET_SCORE_THRESHOLD, dest="score_threshold")
    parser.add_argument("--output-name", default="logits", dest="output_name")
    score_mode = parser.add_mutually_exclusive_group()
    score_mode.add_argument("--single-output-mode", choices=("logit", "prob"), default=DEFAULT_BUCKET_SINGLE_OUTPUT_MODE, dest="single_output_mode",
        help="Scalar ONNX-output interpretation. 'logit' applies sigmoid before thresholding.")
    score_mode.add_argument("--is-logit", action="store_const", const="logit", dest="single_output_mode", help="alias for --single-output-mode logit.")
    score_mode.add_argument("--is-prob", action="store_const", const="prob", dest="single_output_mode", help="alias for --single-output-mode prob.")
    parser.set_defaults(nEvents = -1)

    parser.set_defaults(outRootFile="MsTrkTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON = "perfmonmt_MuonR4Reco.json"
    flags.Trigger.Muon.useNewRegionSelector = False
    
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    if args.use_cpu:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    else:
        flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CUDA

    flags, cfg = setupGeoR4TestCfg(args,flags)

    from MuonConfig.ReconstructionConfigR4 import MuonReconstructionConfig
    cfg.merge(MuonReconstructionConfig(flags))
    
    if not args.skip_onnx:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                ModelPath=args.bucket_model_path,
                ScoreThreshold=args.score_threshold,
                OutputName=args.output_name,
                SingleOutputMode=args.single_output_mode,
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                InferenceTools=[bucketTool],
            )
        )
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"

    #### Schedule the legacy MS track building to compare the two reconstruction chains
    from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg

    if args.LegacyChain:
        cfg.merge(LegacyMuonRecoChainCfg(flags))

    from MuonTrackFindingTest.MsTrackFindingTester import MsTrackTesterCfg
    cfg.merge(MsTrackTesterCfg(flags, scheduleLegacy = args.LegacyChain, outFile = args.outRootFile))

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg


    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                            VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))


    if not args.noMonitorPlots:
        from MuonTrackFindingTest.MsTrackFindingTester import MsTrackVisualizationToolCfg
        cfg.getEventAlgo("MSTrackFinderAlg").VisualizationTool = cfg.popToolsAndMerge(MsTrackVisualizationToolCfg(flags))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags,
                                                                                            CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots",
                                                                                            displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))

        cfg.getEventAlgo("MuonNswSegmentFinderAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags,
                                                                                            CanvasPreFix="NswSegmentFitPlotValid", outSubDir="SegmentValidPlots",
                                                                                            doPhiBucketViews = False, saveSinglePDFs = False,
                                                                                            saveSummaryPDF= False,CanvasLimits=10000))


    cfg.getService("MessageSvc").setVerbose = []
    executeTest(cfg)