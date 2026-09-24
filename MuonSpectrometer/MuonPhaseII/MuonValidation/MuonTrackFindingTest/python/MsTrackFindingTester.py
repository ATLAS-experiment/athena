# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MsTrackTesterCfg(flags, name = "MsTrackTester", scheduleLegacy = True, 
                     outFile="MsTrkTester.root", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    
    result.merge(setupHistSvcCfg(flags, outFile=outFile,
                                 outStream="MuonTrackTester"))

    from MuonTrackFindingAlgs.TrackFindingConfig import SegmentSelectorCfg, TrackSummaryToolCfg, MsTrackSeedingToolCfg, MSExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    kwargs.setdefault("SeedingTool", result.popToolsAndMerge(MsTrackSeedingToolCfg(flags)))
    kwargs.setdefault("storeIdTrks", flags.Reco.EnableTracking)
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
    from MuonTrackFindingAlgs.TrackFindingConfig import MSExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    from MuonTrackFindingAlgs.TrackFindingConfig import MsTrackSeedingToolCfg
    kwargs.setdefault("SeedingTool", result.popToolsAndMerge(MsTrackSeedingToolCfg(flags)))

    the_tool = CompFactory.MuonValR4.TrackVisualizationTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result    

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.add_argument("--writeSpacePoints", help="If set to true, the spacepoints in the bucket are saved to disk",
                                              default=False, action='store_true')
    parser.add_argument("--noPerfMon", help="If set to true, disable performance monitoring.",
                                              default=False, action='store_true')
    parser.add_argument("--noLegacyChain", help="If set to true, the legacy chain is not scheduled",
                                           default = False, action = 'store_true')
    parser.set_defaults(nEvents = -1)
  
    parser.set_defaults(outRootFile="MsTrkTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R3)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.Trigger.Muon.useNewRegionSelector = False
    flags.Muon.scheduleActsReco = True
    flags.Muon.includePileUpTruth = True
    
    from ActsConfig.ActsConfigFlags import TrackFitterType
    if False: flags.Muon.TrackFitterType = TrackFitterType.KalmanFitter
    if False: flags.Muon.trackGeometryMaterialMap = MuonPhaseIITestDefaults.TRKGEO_MATERIALMAP

    flags, cfg = setupGeoR4TestCfg(args,flags)

    cfg.getService("MessageSvc").setVerbose = ["MSTrackFinderAlg"]
    cfg.getService("MessageSvc").setVerbose = []
    from MuonConfig.ReconstructionConfigR4 import MuonReconstructionConfig
    cfg.merge(MuonReconstructionConfig(flags))

    
    #### Schedule the legacy MS track building to compare the two reconstruction chains
    from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg

    if not args.noLegacyChain:
        cfg.merge(LegacyMuonRecoChainCfg(flags))

    cfg.merge(MsTrackTesterCfg(flags, scheduleLegacy = not args.noLegacyChain,
                                      outFile = args.outRootFile))

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg

    
    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                          VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))

    
    if not args.noMonitorPlots:
        cfg.getEventAlgo("MSTrackFinderAlg").VisualizationTool = cfg.popToolsAndMerge(MsTrackVisualizationToolCfg(flags))
        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                            CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots", 
                                                                                            displayTruthOnly = True, saveSinglePDFs = False, saveSummaryPDF= True))
    
        cfg.getEventAlgo("MuonNswSegmentFinderAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                            CanvasPreFix="NswSegmentFitPlotValid", outSubDir="SegmentValidPlots",
                                                                                            doPhiBucketViews = False, saveSinglePDFs = False, 
                                                                                            saveSummaryPDF= True,CanvasLimits=10000))
        
        cfg.getEventAlgo("MuonEtaHoughTransformAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="EtaHoughPlotValid", doPhiBucketViews = False,
                                                                                                outSubDir="EtaHoughiDiPuffPlots", displayTruthOnly = True, 
                                                                                                saveSinglePDFs = True, saveSummaryPDF= True))

        cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots", 
                                                                                                displayTruthOnly = True, saveSinglePDFs = True, saveSummaryPDF= True))
 
    

# 


    executeTest(cfg)
