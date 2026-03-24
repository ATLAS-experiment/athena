# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FastRecoVisualizationToolCfg(flags, name="FastRecoVisualizationTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    kwargs.setdefault("PrdContainer", PrimaryMeasContNamesCfg(flags))
    if flags.Input.isMC:
        from MuonObjectMarker.ObjectMarkerConfig import TruthMeasMarkerAlgCfg
        markerAlg = result.getPrimaryAndMerge(TruthMeasMarkerAlgCfg(flags))
        kwargs.setdefault("TruthSegDecors", [markerAlg.SegmentLinkKey])
        kwargs["TruthSegDecors"] += [markerAlg.SegmentLinkKey]
    the_tool = CompFactory.MuonValR4.FastRecoVisualizationTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonFastRecoTesterCfg(flags, name = "MuonFastRecoTester", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)

    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        kwargs.setdefault("SpacePointKey", "MuonSpacePoints")
    else: 
        kwargs.setdefault("SpacePointKey", "")
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        kwargs.setdefault("NswSpacePointKey", "NswSpacePoints")
    else:
        kwargs.setdefault("NswSpacePointKey", "")

    theAlg = CompFactory.MuonValR4.MuonFastRecoTester(name, **kwargs) 
    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.add_argument("--noPerfMon", help="If set to true, full perfmonMT is enabled",
                                              default=False, action='store_true')
    parser.add_argument("--writeSpacePoints", help="If set to true, the spacepoints in the bucket are saved to disk",
                                              default=False, action='store_true')
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", default = False,
                                            action='store_true')
    parser.add_argument("--runHoughTest", help="If set to true, the hough transform test is run on the output of the fast reco alg",
                                              default=False, action='store_true')
    parser.add_argument("--vTune", help="If set to true, the code is profiled with VTune (With the proper command!)",
                                              default=False, action='store_true')
    parser.set_defaults(outRootFile="FastRecoTester.root")
    from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
    parser.set_defaults(inputFile = MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(eventPrintoutLevel = 50)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    flags.PerfMon.OutputJSON="perfmonmt_MuonR4FastReco.json"
    flags.PerfMon.VTune.ProfiledAlgs = ["MuonFastReconstructionAlg"]

    flags, cfg = setupGeoR4TestCfg(args,flags)
  
    #cfg.getService("MessageSvc").setVerbose = ["MuonFastReconstructionAlg"]
    if args.vTune:
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags))
        
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonFastRecoAlgs.MuonFastReconstructionConfig import MuonFastReconstructionAlgCfg, PatternRecognitionFromFastRecoCfg
    cfg.merge(MuonFastReconstructionAlgCfg(flags))

    if args.runHoughTest:
        cfg.merge(PatternRecognitionFromFastRecoCfg(flags))

        from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg
        cfg.merge(MuonHoughTransformTesterCfg(flags,  
                                              name = "MuonHoughTransformTester", 
                                              SpacePointKey = "MuonSpacePointsFastReco",
                                              writeSpacePoints = False,
                                              VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))
    else:
        from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
        cfg.merge(MuonPatternRecognitionCfg(flags))
    
    cfg.merge(MuonFastRecoTesterCfg(flags, 
                                    name = "MuonFastRecoTester",
                                    writeSpacePoints = args.writeSpacePoints))

    if flags.Input.isMC:
        ## Keep them to manually exchange the map
        # "MDTTwinMapping_compactFormat_allBO", "MDTTwinMapping_compactFormat_fullSpectrometer",  
        # "MDTTwinMapping_compactFormat_Run123",  
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        cfg.merge(addOverride(flags, "/MDT/TWINMAPPING", "MDTTwinMapping_compactFormat_Run123"))

    if not args.noMonitorPlots:
        cfg.getEventAlgo("MuonFastReconstructionAlg").VisualizationTool = cfg.popToolsAndMerge(FastRecoVisualizationToolCfg(flags, 
                                                                                                CanvasPreFix="FastRecoValid", 
                                                                                                doPhiBucketViews = False,
                                                                                                doEtaBucketViews = False,
                                                                                                doRZBucketViews = True,
                                                                                                paintTruthSegment = False,
                                                                                                outSubDir="FastReconstructionValidPlots", 
                                                                                                displayTruthOnly = False,
                                                                                                saveSinglePDFs = True))
    executeTest(cfg)