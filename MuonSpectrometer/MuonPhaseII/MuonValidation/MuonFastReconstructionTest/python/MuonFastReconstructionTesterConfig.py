# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FastRecoVisualizationToolCfg(flags, name="FastRecoVisualizationTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    kwargs.setdefault("PrdContainer", PrimaryMeasContNamesCfg(flags))
    if flags.Muon.setupTruthAlgorithms:
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
    kwargs.setdefault("isSeededReco", flags.Trigger.doHLT) 

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
    parser.add_argument("--runMSTrackTest", help="If set to true, the MS Track Finding test is run on the output of the fast reco alg",
                                              default=False, action='store_true')
    parser.add_argument("--useFastRecoSpacePoints", help="If set to true, we run the pattern recognition chain on the space points from the fast reco instead of spacepointMaker",
                                              default=False, action='store_true')
    parser.add_argument("--vTune", help="If set to true, the code is profiled with VTune (With the proper command!)",
                                              default=False, action='store_true')
    parser.add_argument('--evtNumber',default=None,nargs="+",type=int,help="specify to select an evtNumber")

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
  
    if args.vTune:
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags))

    # Schedule data preparation and space point formation
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))
    
    # Schedule fast reconstruction alg & the Fast Reco Tester Alg
    from MuonFastRecoAlgs.MuonFastReconstructionConfig import MuonFastReconstructionAlgCfg, PatternRecognitionFromFastRecoCfg
    cfg.merge(MuonFastReconstructionAlgCfg(flags))
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                              outStream="FastRecoTester"))
    cfg.merge(MuonFastRecoTesterCfg(flags, 
                                    name = "MuonFastRecoTester",
                                    writeSpacePoints = args.writeSpacePoints))

    # Schedule the pattern recognition algs either on the space points from the fast reco or from the standard space point maker
    if args.runHoughTest or args.runMSTrackTest:
        if args.useFastRecoSpacePoints:
            cfg.merge(PatternRecognitionFromFastRecoCfg(flags))
        else:
            from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
            cfg.merge(MuonPatternRecognitionCfg(flags))

    # If desired, schedule the hough transform test
    if args.runHoughTest:
        cfg.merge(setupHistSvcCfg(flags,outFile="HoughTransformTester.root",
                                  outStream="MuonEtaHoughTransformTest"))
        from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg
        cfg.merge(MuonHoughTransformTesterCfg(flags,  
                                              name = "MuonHoughTransformTester",
                                              writeSpacePoints = False,
                                              VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))
        
    # If desired, schedule the MS Track Finding test
    if args.runMSTrackTest:
        cfg.merge(setupHistSvcCfg(flags,outFile="MsTrackTester.root",
                                  outStream="MuonTrackTester"))
        from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg
        cfg.merge(MSTrackFinderAlgCfg(flags))
        from MuonTrackFindingTest.MsTrackFindingTester import MsTrackTesterCfg
        cfg.merge(MsTrackTesterCfg(flags, LegacyTrackKey="", LegacyMuonKey="", LegacySegmentKey=""))
    

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
    
    if args.evtNumber is not None:
        mainSeq = "AthAllAlgSeq"
        topSeq = cfg.getSequence("AthAlgEvtSeq")
        algSeq = cfg.getSequence(mainSeq)
        mainSeq = "New" + mainSeq
        # topSeq has three sub-sequencers ... preserve first and last
        topSeq.Members = [topSeq.Members[0],
                          CompFactory.AthSequencer(mainSeq, Sequential=True, ModeOR=False, StopOverride=False),
                          topSeq.Members[-1]]
        cfg.addEventAlgo(CompFactory.EventNumberFilterAlgorithm("EvtNumberFilter",EventNumbers=args.evtNumber),sequenceName=mainSeq)
        cfg.getSequence(mainSeq).Members += [algSeq]

        cfg.getService("MessageSvc").setVerbose = ["MuonFastReconstructionAlg", "MuonFastRecoTester"]
    
    executeTest(cfg)