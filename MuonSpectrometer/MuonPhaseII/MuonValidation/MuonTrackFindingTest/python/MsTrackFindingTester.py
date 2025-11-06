# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MsTrackTesterCfg(flags, name = "MsTrackTester", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    from MuonTrackFindingAlgs.TrackFindingConfig import SegmentSelectorCfg, TrackSummaryToolCfg
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
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
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.add_argument("--writeSpacePoints", help="If set to true, the spacepoints in the bucket are saved to disk",
                                              default=False, action='store_true')
    parser.set_defaults(nEvents = -1)
  
    parser.set_defaults(outRootFile="MsTrkTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags.Muon.doFastMMDigitization = True
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags, cfg = setupGeoR4TestCfg(args,flags)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonTrackTester"))

    # cfg.getService("MessageSvc").setVerbose = ["MsTrackTester", "MSTrackFinderAlg", "MuonSegmentFittingAlg"]
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg
    cfg.merge(MSTrackFinderAlgCfg(flags))
    
    ### Schedule the legacy MS track building to compare the two reconstruction chains
    ### from MuonPatternRecognitionTest.PatternTestConfig import LegacyMuonRecoChainCfg
    ### cfg.merge(LegacyMuonRecoChainCfg(flags))

    cfg.merge(MsTrackTesterCfg(flags))

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

    

# 


    executeTest(cfg)
