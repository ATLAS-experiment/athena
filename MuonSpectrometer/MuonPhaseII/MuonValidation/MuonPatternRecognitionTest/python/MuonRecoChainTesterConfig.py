# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaborationation

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonRecoChainTesterCfg(flags,name="MuonRecoChainTester", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    theAlg = CompFactory.MuonValR4.MuonRecoChainTester(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result

if __name__=="__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(outRootFile="HoughTransformTester.root")
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2023-02")
    parser.set_defaults(inputFile=[
                                   "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R3SimHits.pool.root"
                                   # "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    ])
    parser.set_defaults(eventPrintoutLevel = 500)
    parser.add_argument("--displayFailedSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)
    parser.add_argument("--displayGoodSeeds", 
                        help="Saves the hits of failed seeds in a pdf", action='store_true', default = False)
    parser.add_argument("--runVtune", 
                        help="runs VTune profiler service for the muon hough alg", action='store_true', default = False)


    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # flags.PerfMon.doFullMonMT = True
    flags, cfg = setupGeoR4TestCfg(args,flags)
    

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))      
    cfg.merge(MuonSegmentFittingAlgCfg(flags))
    
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg, MuonSegmentFinderAlgCfg, MuonSegmentCnvAlgCfg
    ### Build segments from the leagcy chain
    cfg.merge(MuonLayerHoughAlgCfg(flags))
    cfg.merge(MuonSegmentFinderAlgCfg(flags,
                                      NSWSegmentCollectionName=""))

    cfg.merge(MuonSegmentCnvAlgCfg(flags,
                                   xAODContainerName="MuonSegments"))
    from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg
    cfg.merge(MuPatTrackBuilderCfg(flags))
    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
    cfg.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags))
    
    ### What happens if you parse the R4 patterns to the legacy chain?
    from MuonPatternCnv.MuonPatternCnvConfig import MuonPatternCnvAlgCfg
    cfg.merge(MuonPatternCnvAlgCfg(flags,
                                   PatternCombiKey="R4HoughPatterns",
                                   HoughDataPerSecKey="R4HoughDataPerSec"))
    cfg.merge(MuonSegmentFinderAlgCfg(flags,
                                      name="MuonSegmentFinderR4Pattern",
                                      MuonLayerHoughCombisKey="R4HoughPatterns",
                                      SegmentCollectionName="TrkMuonSegmentsFromHoughR4",
                                      NSWSegmentCollectionName=""))
    cfg.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlgFromHoughR4",
                                   SegmentContainerName="TrkMuonSegmentsFromHoughR4",
                                   xAODContainerName="MuonSegmentsFromHoughR4"))
    
    cfg.merge(MuPatTrackBuilderCfg(flags, name="TrackBuildingFromHoughR4",
                                   MuonSegmentCollection = "TrkMuonSegmentsFromHoughR4",
                                   SpectrometerTrackOutputLocation="MuonTracksFromHoughR4"))
    cfg.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,"MuonXAODParticleConvFromHoughR4",
                                                   TrackContainerName="MuonTracksFromHoughR4",
                                                   xAODTrackParticlesFromTracksContainerName="MuonSpectrometerTrackParticlesFromHoughR4"))

    ### Convert the R4 segments into the legacy format
    from MuonSegmentCnv.MuonSegmentCnvConfig import MuonR4SegmentCnvAlgCfg
    cfg.merge(MuonR4SegmentCnvAlgCfg(flags))
    
    cfg.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlgR4Chain",
                                   SegmentContainerName="TrackMuonSegmentsR4",
                                   xAODContainerName="MuonSegmentsFromR4"))

    cfg.merge(MuPatTrackBuilderCfg(flags, name="TrackBuildingFromR4Segments",
                                   MuonSegmentCollection = "TrackMuonSegmentsR4",
                                   SpectrometerTrackOutputLocation="MuonTracksR4"))
    cfg.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,"MuonXAODParticleConvR4",
                                                   TrackContainerName="MuonTracksR4",
                                                   xAODTrackParticlesFromTracksContainerName="MuonSpectrometerTrackParticlesR4"))



    cfg.merge(MuonRecoChainTesterCfg(flags))
    
    if args.runVtune: 
        from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
        cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))

    executeTest(cfg)
    
