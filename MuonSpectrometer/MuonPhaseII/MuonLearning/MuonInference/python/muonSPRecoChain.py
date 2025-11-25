# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaborationation

if __name__=="__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(outRootFile="RecoChainTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    
    parser.add_argument("--monitorPlots", action='store_true', default=False, 
                        help="Setup monitoring plots of the pattern recognition")
    parser.add_argument("--runVtune", 
                        help="runs VTune profiler service for the muon hough alg", action='store_true', default = False)
    parser.add_argument("--noPerfMon", help="If set to true, full perfmonMT is enabled",
                        default=False, action='store_true')
    parser.add_argument("--use-gpu", action="store_true", default=True, 
                       help="Use GPU for ONNX inference (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                       help="Use CPU for ONNX inference")
    parser.add_argument("--skip_bucket_filter", action="store_true", default=False,
                       help="Skip the bucket filter inference step (default: False)")

    args = parser.parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"
    
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    # Determine whether user requested GPU (parser sets args.use_gpu)
    use_gpu_requested = getattr(args, "use_gpu", True)
    # Runtime check for GPU availability. Prefer ONNXRuntime provider list,
    # fall back to PyTorch if ONNX runtime isn't available.
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

    flags, cfg = setupGeoR4TestCfg(args,flags)
    

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    ###### ML Inference

    ### SP Filter (alone)
    #from MuonInference.InferenceConfig import GraphSPFilterToolCfg, GraphInferenceAlgCfg
    #cfg.merge(GraphInferenceAlgCfg(flags,InferenceTools = [cfg.popToolsAndMerge(GraphSPFilterToolCfg(flags))]))

    ### Bucket + SP Filter
    from MuonInference.InferenceConfig import (
        GraphBucketFilterToolCfg,
        GraphSPFilterToolCfg,
        GraphInferenceAlgCfg,
    )
    # 1) Bucket filter: read raw SPs, write filtered buckets (optional)
    if not args.skip_bucket_filter:
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                ReadSpacePoints="MuonSpacePoints",
                WriteSpacePointKey="FilteredMlBuckets",
            )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                name="GraphInferenceAlgBuckets",
                InferenceTools=[bucketTool],
            )
        )
        # SP filter reads from bucket filter output
        sp_input_container = "FilteredMlBuckets"
    else:
        # Skip bucket filter, SP filter reads directly from raw space points
        sp_input_container = "MuonSpacePoints"
    
    # 2) SP filter: read filtered buckets (or raw SPs), write filtered SPs
    spTool = cfg.popToolsAndMerge(
        GraphSPFilterToolCfg(
            flags,
            ReadSpacePoints=sp_input_container,         # chain from previous stage or raw SPs
            WriteSpacePointKey="FilteredMlSpacePoints",  # final SPs for Hough
            # MLFilterCut=-3.6,
        )
    )
    cfg.merge(
        GraphInferenceAlgCfg(
            flags,
            name="GraphInferenceAlgSP",    # unique name
            InferenceTools=[spTool],
        )
    )

    ### Build segments from the legacy chain
    from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg
    cfg.merge(LegacyMuonRecoChainCfg(flags))

    ### Setup the new chain
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags)) 
    
    cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlSpacePoints"


    from MuonPatternRecognitionTest.PatternTestConfig import MuonR4PatternRecoChainCfg, MuonR4SegmentRecoChainCfg
    cfg.merge(MuonR4PatternRecoChainCfg(flags))

    ### What happens if you parse the R4 patterns to the legacy chain?
    cfg.merge(MuonR4SegmentRecoChainCfg(flags))

    from MuonPatternRecognitionTest.PatternTestConfig import TrackTruthMatchCfg
    cfg.merge(TrackTruthMatchCfg(flags))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonRecoChainTesterCfg
    cfg.merge(MuonRecoChainTesterCfg(flags))
    if args.runVtune: 
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))
    
    if args.monitorPlots:
        from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="EtaHoughPlotValid",
                                                                                                AllCanvasName="AllEtaHoughiDiPuffPlots", doPhiBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonPhiHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="PhiHoughPlotValid",
                                                                                                AllCanvasName="AllPhiHoughiDiPuffPlots",doEtaBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= False))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="SegmentPlotValid",
                                                                                                AllCanvasName="AllSegmentFitPlots", doPhiBucketViews = False,
                                                                                                displayTruthOnly = True, saveSinglePDFs = True, saveSummaryPDF= False))
    executeTest(cfg)


