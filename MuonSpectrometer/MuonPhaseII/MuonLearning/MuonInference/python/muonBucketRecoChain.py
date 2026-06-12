# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import logging

# Suppress ONNX Runtime warnings at Python logging level before Athena initialization
logging.getLogger("onnxruntime").setLevel(logging.ERROR)

# Set environment variable for ONNX Runtime before imports (attempt early suppression)
os.environ["ORT_LOGGING_LEVEL"] = "3"  # 3 = ERROR

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MsTrackTesterCfg(flags, name = "MsTrackTester", scheduleLegacy = True, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    from MuonTrackFindingAlgs.TrackFindingConfig import SegmentSelectorCfg, TrackSummaryToolCfg
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    if not scheduleLegacy:
        kwargs.setdefault("LegacySegmentKey", "")
        kwargs.setdefault("LegacyTrackKey", "")
        kwargs.setdefault("LegacyMuonKey" , "")
    the_alg = CompFactory.MuonValR4.MsTrackTester(name= name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MsTrackVisualizationToolCfg(flags, name = "VisualizationTool", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg
        result.merge(LegacyMuonRecoChainCfg(flags))
        kwargs.setdefault("TruthSegkey", "MuonSegments")
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)))
    the_tool = CompFactory.MuonValR4.TrackVisualizationTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

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
    parser.add_argument("--bucket-model-path", dest="bucket_model_path", default="dev/MuonRecRTT/edgecnn_mu200.onnx")
    parser.add_argument("--score-threshold", type=float, default=0.160, dest="score_threshold")
    parser.add_argument("--output-name", default="logits", dest="output_name")
    parser.add_argument("--graph-bucket-output-level", type=int, default=3, dest="graph_bucket_output_level", help="OutputLevel for GraphBucketFilterTool")
    parser.add_argument("--is-logit", dest="is_logit", default=False, action="store_true", help="Interpret the single output directly and do not apply sigmoid")
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

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonTrackTester"))

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
                OutputLevel=args.graph_bucket_output_level,
                SingleOutputIsLogit=args.is_logit if hasattr(args, "is_logit") else False,
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

    cfg.merge(MsTrackTesterCfg(flags, scheduleLegacy = args.LegacyChain))

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg


    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                            VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))


    if not args.noMonitorPlots:
        cfg.getEventAlgo("MSTrackFinderAlg").VisualizationTool = cfg.popToolsAndMerge(MsTrackVisualizationToolCfg(flags))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags,
                                                                                            CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots",
                                                                                            displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))

        cfg.getEventAlgo("MuonNswSegmentFinderAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags,
                                                                                            CanvasPreFix="NswSegmentFitPlotValid", outSubDir="SegmentValidPlots",
                                                                                            doPhiBucketViews = False, saveSinglePDFs = False,
                                                                                            saveSummaryPDF= False,CanvasLimits=10000))



    executeTest(cfg)
