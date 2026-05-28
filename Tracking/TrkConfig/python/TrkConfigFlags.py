# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod, FlagEnum


class TrackFitterType(FlagEnum):
    DistributedKalmanFilter = 'DistributedKalmanFilter'
    GlobalChi2Fitter = 'GlobalChi2Fitter'
    GaussianSumFilter = 'GaussianSumFilter'


class KalmanUpdatorType(FlagEnum):
    KalmanUpdator = 'KalmanUpdator'
    KalmanUpdator_xk = 'KalmanUpdator_xk'
    KalmanUpdatorSMatrix = 'KalmanUpdatorSMatrix'
    KalmanWeightUpdator = 'KalmanWeightUpdator'
    KalmanUpdatorAmg = 'KalmanUpdatorAmg'


class PixelClusterSplittingType(FlagEnum):
    NeuralNet = 'NeuralNet'
    Truth = 'Truth'


class TrackingComponent(FlagEnum):
    AthenaChain = "AthenaChain"  # full Athena Chain (default)
    ActsChain = "ActsChain"  # full Acts Chain 
    # Validation options
    ValidateActsClusters = "ValidateActsClusters"
    ValidateActsSpacePoints = "ValidateActsSpacePoints" 
    ValidateActsSeeds = "ValidateActsSeeds"
    ValidateActsTracks = "ValidateActsTracks"


def createTrackingConfigFlags():
    icf = AthConfigFlags()

    # Turn running of truth matching on and off (by default on for MC off for data)
    icf.addFlag("Tracking.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # control which fitter to be used
    icf.addFlag("Tracking.trackFitterType",
                TrackFitterType.GlobalChi2Fitter, enum=TrackFitterType)
    # control which measurement updator to load as InDetUpdator
    icf.addFlag("Tracking.kalmanUpdator",
                KalmanUpdatorType.KalmanUpdatorSMatrix, enum=KalmanUpdatorType)

    icf.addFlag("Tracking.materialInteractions", lambda prevFlags:
                prevFlags.Beam.Type is not BeamType.SingleBeam)
    # Control which type of particle hypothesis to use for the material interactions
    # 0=non-interacting,1=electron,2=muon,3=pion,4=kaon,5=proton. See ParticleHypothesis.h for full definition.
    icf.addFlag("Tracking.materialInteractionsType", lambda prevFlags:
                2 if prevFlags.Beam.Type is BeamType.Cosmics else 3)

    # Turn on running of Brem Recovery in tracking
    icf.addFlag("Tracking.doBremRecovery", lambda prevFlags: (
        not (prevFlags.Tracking.doVtxLumi or
             prevFlags.Tracking.doVtxBeamSpot or
             prevFlags.Tracking.doLowMu or
             prevFlags.Beam.Type is not BeamType.Collisions or
             not prevFlags.BField.solenoidOn)))
    # Brem Recover in tracking restricted to Calo ROIs
    icf.addFlag("Tracking.doCaloSeededBrem", lambda prevFlags:
                prevFlags.Detector.EnableCalo)
    # Use Recover SSS to Calo ROIs
    icf.addFlag("Tracking.doHadCaloSeededSSS", False)
    # Use Calo ROIs to seed specific cuts for the ambi
    icf.addFlag("Tracking.doCaloSeededAmbi", lambda prevFlags:
                prevFlags.Detector.EnableCalo)
    # control if the shared hits are recorded in TrackPatricles
    icf.addFlag("Tracking.doSharedHits", True)
    # Switch for running TIDE Ambi
    icf.addFlag("Tracking.doTIDE_Ambi", lambda prevFlags:
                not (prevFlags.Beam.Type is BeamType.Cosmics))
    # Use simple position and error estimate for on-track pixel cluster
    icf.addFlag("Tracking.doPixelDigitalClustering", False)
    # Try to split pixel clusters
    icf.addFlag("Tracking.doPixelClusterSplitting",
                lambda prevFlags: not (prevFlags.Beam.Type is BeamType.Cosmics))
    # choose splitter type: NeuralNet or AnalogClus
    icf.addFlag("Tracking.pixelClusterSplittingType", lambda prevFlags:
                PixelClusterSplittingType.NeuralNet
                if prevFlags.GeoModel.Run <= LHCPeriod.Run3
                else PixelClusterSplittingType.Truth,
                enum=PixelClusterSplittingType)
    # Cut value for splitting clusters into two parts
    icf.addFlag("Tracking.pixelClusterSplitProb1",
                lambda prevFlags: (
                    0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.55))
    # Cut value for splitting clusters into three parts
    icf.addFlag("Tracking.pixelClusterSplitProb2",
                lambda prevFlags: (
                    0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.45))
    # Skip ambiguity solver in hadronic ROI
    icf.addFlag("Tracking.doSkipAmbiROI", False)


    # Express track parameters wrt. to : 'BeamLine','BeamSpot','Vertex' (first primary vertex)
    icf.addFlag("Tracking.perigeeExpression", lambda prevFlags:
                "Vertex" if prevFlags.Reco.EnableHI else "BeamLine")

    # Tracking passes/configurations scheduled

    def doLargeD0(flags):
        if flags.GeoModel.Run <= LHCPeriod.Run3:
            return not ((flags.Beam.Type in
                        [BeamType.SingleBeam, BeamType.Cosmics]) or
                        flags.Reco.EnableHI or
                        flags.Tracking.doHighPileup or
                        flags.Tracking.doVtxLumi or
                        flags.Tracking.doVtxBeamSpot)
        else:  # LRT disabled by default for Run4 for now
            return False

    icf.addFlag("Tracking.doLargeD0", doLargeD0)
    icf.addFlag("Tracking.storeSeparateLargeD0Container", True)

    # Special configuration for low-mu runs
    icf.addFlag("Tracking.doLowMu", False)
    # Turn running of doLowPt second pass on and off
    icf.addFlag("Tracking.doLowPt",
                lambda prevFlags: prevFlags.Tracking.doLowMu)

    # Turn on to save the Track Seeds in a xAOD track collecting for development studies
    icf.addFlag("Tracking.doStoreTrackSeeds", False)
    # Save SiSP tracks (input to the ambiguity solver)
    icf.addFlag("Tracking.doStoreSiSPSeededTracks", False)
    # Turn writing of seed validation ntuple on and off
    icf.addFlag("Tracking.writeSeedValNtuple", False)
    # Save xAOD TrackMeasurementValidation + TrackStateValidation containers
    icf.addFlag("Tracking.writeExtendedPRDInfo", False)

    # Toggle track slimming
    icf.addFlag("Tracking.doSlimming", lambda prevFlags:
                not ((prevFlags.Beam.Type in
                     [BeamType.SingleBeam, BeamType.Cosmics]) or
                     prevFlags.Tracking.doHighPileup or
                     prevFlags.Tracking.doVtxLumi))

    ####################################################################

    # The following flags are only used in InDet configurations for now
    # No corresponding ITk config is available yet

    def cutLevel(flags):
        if flags.Reco.EnableHI:
            return 2
        elif flags.Tracking.doLowMu:
            return 3
        elif flags.Beam.Type is BeamType.Cosmics:
            return 8
        elif flags.Tracking.doMinBias:
            return 12
        else:
            return 19

    # Control cuts and settings for different lumi to limit CPU and disk space
    icf.addFlag("Tracking.cutLevel", cutLevel)

    # Turn on InDetRecStatistics
    icf.addFlag("Tracking.doStats", False)
    # Switch for track observer tool
    icf.addFlag("Tracking.doTIDE_AmbiTrackMonitoring", False)
    # use beam spot position in pixel NN
    icf.addFlag("Tracking.useBeamSpotInfoNN", True)
    # Threshold for NN cut in large D0 tracking for tracks in ambi
    icf.addFlag("Tracking.nnCutLargeD0Threshold", -1.0)
    # Use broad cluster errors for Pixel
    icf.addFlag("Tracking.useBroadPixClusterErrors", False)
    # Use broad cluster errors for SCT
    icf.addFlag("Tracking.useBroadSCTClusterErrors", False)

    # Tracking passes/configurations scheduled

    # Turn running of track segment creation in pixel on and off
    icf.addFlag("Tracking.doTrackSegmentsPixel",
                lambda prevFlags: (
                    prevFlags.Detector.EnablePixel and (
                        prevFlags.Tracking.doMinBias or
                        prevFlags.Tracking.doLowMu or
                        prevFlags.Beam.Type is BeamType.Cosmics)))
    # Turn running of track segment creation in SCT on and off
    icf.addFlag("Tracking.doTrackSegmentsSCT",
                lambda prevFlags: (
                    prevFlags.Detector.EnableSCT and (
                        prevFlags.Tracking.doLowMu or
                        prevFlags.Beam.Type is BeamType.Cosmics)))
    # Turn running of track segment creation in TRT on and off
    icf.addFlag("Tracking.doTrackSegmentsTRT",
                lambda prevFlags: (
                    prevFlags.Detector.EnableTRT and
                    (prevFlags.Tracking.doLowMu or
                     prevFlags.Beam.Type is BeamType.Cosmics)))
    # turn on / off TRT extensions
    icf.addFlag("Tracking.doTRTExtension",
                lambda prevFlags: prevFlags.Detector.EnableTRT)
    # control to run TRT Segment finding (do it always after new tracking!)
    icf.addFlag("Tracking.doTRTSegments",
                lambda prevFlags: (prevFlags.Detector.EnableTRT and
                                   (prevFlags.Tracking.doBackTracking or
                                    prevFlags.Tracking.doTRTStandalone)))
    # Turn running of backtracking on and off
    icf.addFlag("Tracking.doBackTracking", lambda prevFlags: (
        prevFlags.Detector.EnableTRT and
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot)))
    # control TRT Standalone
    icf.addFlag("Tracking.doTRTStandalone", lambda prevFlags: (
        prevFlags.Detector.EnableTRT and
        not(prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot)))

    # Turn running of doForwardTracks pass on and off
    icf.addFlag("Tracking.doForwardTracks", lambda prevFlags: (
        prevFlags.Detector.EnablePixel and
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Reco.EnableHI or
            prevFlags.Tracking.doHighPileup or
            prevFlags.Tracking.doVtxLumi or
            prevFlags.Tracking.doVtxBeamSpot or
            prevFlags.Tracking.doMinBias or
            prevFlags.Tracking.doLowMu)))
    icf.addFlag("Tracking.doTrackSegmentsDisappearing",
                lambda prevFlags: (
                    not(prevFlags.Reco.EnableHI or
                        prevFlags.Beam.Type is BeamType.Cosmics)))

    # Turn running of doVeryLowPt third pass on and off
    icf.addFlag("Tracking.doVeryLowPt", False)
    # Turn running of doLargeD0 second pass down to 100 MeV on and off
    icf.addFlag("Tracking.doLowPtLargeD0", False)
    # Switch for running LowPtRoI settings
    icf.addFlag("Tracking.doLowPtRoI", False)
    # Turn running of high pile-up reconstruction on and off
    icf.addFlag("Tracking.doHighPileup", False)
    # Special reconstruction for vertex lumi measurement
    icf.addFlag("Tracking.doVtxLumi", False)
    # Special reconstruction for vertex beamspot measurement
    icf.addFlag("Tracking.doVtxBeamSpot", False)
    # Switch for running MinBias settings
    icf.addFlag("Tracking.doMinBias", False)
    # Turn running of BeamGas second pass on and off
    icf.addFlag("Tracking.doBeamGas",
                lambda prevFlags: prevFlags.Beam.Type is BeamType.SingleBeam)
    # Switch for running MinBias settings with a 300 MeV pT cut (for Heavy Ion Proton)
    icf.addFlag("Tracking.doHIP300", False)
    # Switch for running Robust settings
    icf.addFlag("Tracking.doRobustReco", False)
    # Special reconstruction for BLS physics
    icf.addFlag("Tracking.doBLS", False)

    # Special pass using truth information for pattern recognition, runs in parallel to/instead of the first pass
    icf.addFlag("Tracking.doPseudoTracking", False)
    # Special pass using truth information for pattern recognition, removes assumed in-efficencies applied to PseudoTracking
    icf.addFlag("Tracking.doIdealPseudoTracking", False)

    ####################################################################

    # The following flags are only used in ITk configurations

    # Turn running of ITk FastTracking on and off
    icf.addFlag("Tracking.doITkFastTracking", False)
    # Turn running of ConversionFinding second pass on and off
    icf.addFlag("Tracking.doITkConversionFinding",True)
    # Allows TrigFastTrackFinder to be run as an offline algorithm by replacing
    # SiSPSeededTrackFinder
    icf.addFlag("Tracking.useITkFTF", False)

    # enable reco steps 
    icf.addFlag("Tracking.recoChain", [TrackingComponent.AthenaChain])

    ####################################################################

    # Tracking pass flags

    # InDet

    from TrkConfig.TrackingPassFlags import (
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
    icf.addFlagsCategory("Tracking.MainPass",
                         createTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.HighPileupPass",
                         createHighPileupTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.MinBiasPass",
                         createMinBiasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LargeD0Pass",
                         createLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.R3LargeD0Pass",
                         createR3LargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtLargeD0Pass",
                         createLowPtLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtPass",
                         createLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VeryLowPtPass",
                         createVeryLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtRoIPass",
                         createLowPtRoITrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.ForwardPass",
                         createForwardTracksTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.BeamGasPass",
                         createBeamGasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VtxLumiPass",
                         createVtxLumiTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VtxBeamSpotPass",
                         createVtxBeamSpotTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.CosmicsPass",
                         createCosmicsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.HeavyIonPass",
                         createHeavyIonTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.PixelPass",
                         createPixelTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.DisappearingPass",
                         createDisappearingTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.SCTPass",
                         createSCTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.TRTPass",
                         createTRTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.TRTStandalonePass",
                         createTRTStandaloneTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.RobustRecoPass",
                         createRobustRecoTrackingPassFlags, prefix=True)

    # ITk

    from TrkConfig.TrackingPassFlags import (
        createITkTrackingPassFlags, createITkLargeD0TrackingPassFlags,
        createITkConversionFindingTrackingPassFlags,
        createITkFastTrackingPassFlags, createITkLargeD0FastTrackingPassFlags,
        createITkFTFPassFlags, createITkLowPtTrackingPassFlags)

    icf.addFlagsCategory ("Tracking.ITkMainPass",
                          createITkTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLargeD0Pass",
                          createITkLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkConversionFindingPass",
                          createITkConversionFindingTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLowPt",
                          createITkLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkFastPass",
                          createITkFastTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLargeD0FastPass",
                          createITkLargeD0FastTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkFTFPass",
                          createITkFTFPassFlags, prefix=True)

    ####################################################################

    # Vertexing flags
    from TrkConfig.VertexFindingFlags import (
        createSecVertexingFlags, createEGammaPileUpSecVertexingFlags,
        createPriVertexingFlags)
    icf.addFlagsCategory("Tracking.PriVertex",
                         createPriVertexingFlags, prefix=True)
    icf.addFlagsCategory("Tracking.SecVertex",
                         createSecVertexingFlags, prefix=True)
    icf.addFlagsCategory("Tracking.SecVertexEGammaPileUp",
                         createEGammaPileUpSecVertexingFlags, prefix=True)

    # Turn on the primary vertex reconstruction
    icf.addFlag("Tracking.doVertexFinding",
                lambda prevFlags: prevFlags.Beam.Type is not BeamType.Cosmics)

    return icf
