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
    parser.add_argument("--houghR4", help="Schedules the R4 pattern -> legacy segment -> legacy track chain",
                        action="store_true", default = False)
    parser.add_argument("--use-gpu", action="store_true", dest="use_gpu", default=None,
                       help="Use GPU for ONNX inference (default: True)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                       help="Use CPU for ONNX inference")
    parser.add_argument("--skip-onnx", action="store_true", default=False,
                       help="Skip ONNX inference step")
  
    args = parser.parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4Reco.json"

    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    # Use command line argument if provided, otherwise default to True
    use_gpu_requested = args.use_gpu if args.use_gpu is not None else True
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
    
    ### Build segments from the leagcy chain
    from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg
    cfg.merge(LegacyMuonRecoChainCfg(flags))

    if not args.skip_onnx:
        ### Setup the ONNX inference step
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = cfg.popToolsAndMerge(
            GraphBucketFilterToolCfg(
                flags,
                )
        )
        cfg.merge(
            GraphInferenceAlgCfg(
                flags,
                InferenceTools=[bucketTool],
            )
        )

    ### Setup the new chain
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))    
    
    if not args.skip_onnx:
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlBuckets"

    from MuonPatternRecognitionTest.PatternTestConfig import MuonR4PatternRecoChainCfg, MuonR4SegmentRecoChainCfg
    if args.houghR4:
        cfg.merge(MuonR4PatternRecoChainCfg(flags))

    ### What happens if you parse the R4 patterns to the legacy chain?
    cfg.merge(MuonR4SegmentRecoChainCfg(flags))

    from MuonPatternRecognitionTest.PatternTestConfig import TrackTruthMatchCfg
    cfg.merge(TrackTruthMatchCfg(flags, setupHoughR4 = args.houghR4))


    from MuonPatternRecognitionTest.PatternTestConfig import MuonRecoChainTesterCfg
    cfg.merge(MuonRecoChainTesterCfg(flags,
                                    SegmentFromR4HoughKey = "MuonSegmentsFromHoughR4" if args.houghR4 else "" ))

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


