# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod

def createInDetConfigFlags():
    icf = AthConfigFlags()

    # Detector flags
    # Turn running of the truth seeded pseudo tracking only for pileup on and off.
    # Only makes sense to run on RDO file where SplitDigi was used!
    icf.addFlag("InDet.doSplitReco", False)
    # Turn on running of PRD MultiTruthMaker
    icf.addFlag("InDet.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # defines if the X1X mode is used for the offline or not
    icf.addFlag("InDet.selectSCTIntimeHits", lambda prevFlags: (
        not(prevFlags.Beam.Type is BeamType.Cosmics or \
            prevFlags.Tracking.doVtxBeamSpot)))
    icf.addFlag("InDet.useDCS", True)
    icf.addFlag("InDet.usePixelDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnablePixel))
    icf.addFlag("InDet.useSctDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnableSCT))
    # Use old (non CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoraCool", False)
    # Use new (CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoolVectorPayload", False)
    # Turn on SCT_ModuleVetoSvc, allowing it to be configured later
    icf.addFlag("InDet.doSCTModuleVeto", False)
    # Enable check for dead modules and FEs
    icf.addFlag("InDet.checkDeadElementsOnTrack", True)
    # Turn running of Event Info TRT Occupancy Filling Alg on and off (also whether it is used in TRT PID calculation)
    icf.addFlag("InDet.doTRTGlobalOccupancy", False)
    icf.addFlag("InDet.noTRTTiming",
                lambda prevFlags: prevFlags.Beam.Type is BeamType.SingleBeam)
    icf.addFlag("InDet.doTRTPhase",
                lambda prevFlags: prevFlags.Beam.Type is BeamType.Cosmics)

    # Tracking parameters

    # Turn on running of Brem Recovery in tracking
    icf.addFlag("InDet.Tracking.doBremRecovery", lambda prevFlags: (
        not (prevFlags.Tracking.doVtxLumi or
             prevFlags.Tracking.doVtxBeamSpot or
             prevFlags.Tracking.doLowMu or
             prevFlags.Beam.Type is not BeamType.Collisions or
             not prevFlags.BField.solenoidOn)))
    # Brem Recover in tracking restricted to Calo ROIs
    icf.addFlag("InDet.Tracking.doCaloSeededBrem", True)
    # Use Recover SSS to Calo ROIs
    icf.addFlag("InDet.Tracking.doHadCaloSeededSSS", False)
    # Use Calo ROIs to seed specific cuts for the ambi
    icf.addFlag("InDet.Tracking.doCaloSeededAmbi",
                lambda prevFlags: prevFlags.Detector.EnableCalo)
    # Try to split pixel clusters
    icf.addFlag("InDet.Tracking.doPixelClusterSplitting",
                lambda prevFlags: not(prevFlags.Beam.Type is BeamType.Cosmics))
    # choose splitter type: NeuralNet or AnalogClus
    icf.addFlag("InDet.Tracking.pixelClusterSplittingType", "NeuralNet")
    # Cut value for splitting clusters into two parts
    icf.addFlag("InDet.Tracking.pixelClusterSplitProb1",
                lambda prevFlags: (0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.55))
    # Cut value for splitting clusters into three parts
    icf.addFlag("InDet.Tracking.pixelClusterSplitProb2",
                lambda prevFlags: (0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.45))
    # use beam spot position in pixel NN
    icf.addFlag("InDet.Tracking.useBeamSpotInfoNN", True)
    # Enable check for dead modules and FEs
    icf.addFlag("InDet.Tracking.nnCutLargeD0Threshold", -1.0)
    icf.addFlag("InDet.Tracking.useBroadPixClusterErrors",
                False)  # Use broad cluster errors for Pixel
    icf.addFlag("InDet.Tracking.useBroadSCTClusterErrors",
                False)  # Use broad cluster errors for SCT

    # Tracking passes/configurations scheduled

    # Turn running of track segment creation in pixel on and off
    icf.addFlag("InDet.Tracking.doTrackSegmentsPixel",
                lambda prevFlags: (
                    prevFlags.Tracking.doMinBias or
                    prevFlags.Tracking.doLowMu or
                    prevFlags.Beam.Type is BeamType.Cosmics))
    # Turn running of track segment creation in SCT on and off
    icf.addFlag("InDet.Tracking.doTrackSegmentsSCT",
                lambda prevFlags: (prevFlags.Tracking.doLowMu or
                                   prevFlags.Beam.Type is BeamType.Cosmics))
    # Turn running of track segment creation in TRT on and off
    icf.addFlag("InDet.Tracking.doTrackSegmentsTRT",
                lambda prevFlags: (prevFlags.Tracking.doLowMu or
                                   prevFlags.Beam.Type is BeamType.Cosmics))
    # turn on / off TRT extensions
    icf.addFlag("InDet.Tracking.doTRTExtension", True)
    # control to run TRT Segment finding (do it always after new tracking!)
    icf.addFlag("InDet.Tracking.doTRTSegments",
                lambda prevFlags: (prevFlags.InDet.Tracking.doBackTracking or
                                   prevFlags.InDet.Tracking.doTRTStandalone))
    # Turn running of backtracking on and off
    icf.addFlag("InDet.Tracking.doBackTracking", lambda prevFlags: (
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot)))
    # Turn running of doLowPt second pass on and off
    icf.addFlag("InDet.Tracking.doLowPt",
                lambda prevFlags: prevFlags.Tracking.doLowMu)
    # Turn running of doVeryLowPt thrid pass on and off
    icf.addFlag("InDet.Tracking.doVeryLowPt", False)
    # control TRT Standalone
    icf.addFlag("InDet.Tracking.doTRTStandalone", lambda prevFlags: (
        not(prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot)))
    # Turn running of doForwardTracks pass on and off
    icf.addFlag("InDet.Tracking.doForwardTracks", lambda prevFlags: (
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot or
            prevFlags.Tracking.doMinBias or
            prevFlags.Tracking.doLowMu)))
    icf.addFlag("InDet.Tracking.doTrackSegmentsDisappearing", lambda prevFlags: (
        not(prevFlags.Reco.EnableHI or
            prevFlags.Beam.Type is BeamType.Cosmics)))
    # Turn running of BeamGas second pass on and off
    icf.addFlag("InDet.Tracking.doBeamGas",
                lambda prevFlags: prevFlags.Beam.Type is BeamType.SingleBeam)
    # Switch for running MinBias settings with a 300 MeV pT cut (for Heavy Ion Proton)
    icf.addFlag("InDet.Tracking.doHIP300", False)
    # Switch for running Robust settings
    icf.addFlag("InDet.Tracking.doRobustReco", False)
    # Switch for running looser settings in ID for commissioning
    # Special reconstruction for BLS physics
    icf.addFlag("InDet.Tracking.doBLS", False)
    icf.addFlag("InDet.Tracking.writeSeedValNtuple", False) # Turn writing of seed validation ntuple on and off
    icf.addFlag("InDet.Tracking.writeExtendedPRDInfo", False)
    # Special pass using truth information for pattern recognition, runs in parallel to/instead of the first pass
    icf.addFlag("InDet.Tracking.doPseudoTracking", False)
    # Special pass using truth information for pattern recognition, removes assumed in-efficencies applied to PseudoTracking
    icf.addFlag("InDet.Tracking.doIdealPseudoTracking", False)
    # Save SiSP tracks (input to the ambiguity solver)
    icf.addFlag("InDet.Tracking.doStoreSiSPSeededTracks", False)
    # Skip ambiguity solver in hadronic ROI
    icf.addFlag("InDet.Tracking.doSkipAmbiROI", False)


    from InDetConfig.TrackingPassFlags import (
        createTrackingPassFlags, createHighPileupTrackingPassFlags,
        createMinBiasTrackingPassFlags, createLargeD0TrackingPassFlags,
        createR3LargeD0TrackingPassFlags, createLowPtLargeD0TrackingPassFlags,
        createLowPtTrackingPassFlags, createVeryLowPtTrackingPassFlags, createLowPtRoITrackingPassFlags,
        createForwardTracksTrackingPassFlags, createBeamGasTrackingPassFlags,
        createVtxLumiTrackingPassFlags, createVtxBeamSpotTrackingPassFlags, createCosmicsTrackingPassFlags,
        createHeavyIonTrackingPassFlags, createPixelTrackingPassFlags, createDisappearingTrackingPassFlags,
        createSCTTrackingPassFlags, createTRTTrackingPassFlags, createTRTStandaloneTrackingPassFlags,
        createRobustRecoTrackingPassFlags)

    # Set up for first tracking pass, updated for second passes
    icf.addFlagsCategory("InDet.Tracking.MainPass",
                         createTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.HighPileupPass",
                         createHighPileupTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.MinBiasPass",
                         createMinBiasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LargeD0Pass",
                         createLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.R3LargeD0Pass",
                         createR3LargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LowPtLargeD0Pass",
                         createLowPtLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LowPtPass",
                         createLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VeryLowPtPass",
                         createVeryLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LowPtRoIPass",
                         createLowPtRoITrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.ForwardPass",
                         createForwardTracksTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.BeamGasPass",
                         createBeamGasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VtxLumiPass",
                         createVtxLumiTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VtxBeamSpotPass",
                         createVtxBeamSpotTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.CosmicsPass",
                         createCosmicsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.HeavyIonPass",
                         createHeavyIonTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.PixelPass",
                         createPixelTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.DisappearingPass",
                         createDisappearingTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.SCTPass",
                         createSCTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.TRTPass",
                         createTRTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.TRTStandalonePass",
                         createTRTStandaloneTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.RobustRecoPass",
                         createRobustRecoTrackingPassFlags, prefix=True)

    return icf
