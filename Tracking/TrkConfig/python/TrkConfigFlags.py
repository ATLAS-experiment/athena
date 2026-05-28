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

    # control if the shared hits are recorded in TrackPatricles
    icf.addFlag("Tracking.doSharedHits", True)
    # Switch for running TIDE Ambi
    icf.addFlag("Tracking.doTIDE_Ambi", lambda prevFlags:
                not(prevFlags.Beam.Type is BeamType.Cosmics))

    # Express track parameters wrt. to : 'BeamLine','BeamSpot','Vertex' (first primary vertex)
    icf.addFlag("Tracking.perigeeExpression", lambda prevFlags:
                "Vertex" if prevFlags.Reco.EnableHI else "BeamLine")

    def doLargeD0(flags):
        if flags.GeoModel.Run<=LHCPeriod.Run3:
           return not(flags.Beam.Type in [BeamType.SingleBeam, \
                                          BeamType.Cosmics] or \
                      flags.Reco.EnableHI or \
                      flags.Tracking.doHighPileup or \
                      flags.Tracking.doVtxLumi or \
                      flags.Tracking.doVtxBeamSpot)
        else: # LRT disabled by default for Run4 for now
            return False

    icf.addFlag("Tracking.doLargeD0", doLargeD0)
    icf.addFlag("Tracking.storeSeparateLargeD0Container", True)

    # Special configuration for low-mu runs
    icf.addFlag("Tracking.doLowMu", False)

    # Turn on to save the Track Seeds in a xAOD track collecting for development studies
    icf.addFlag("Tracking.doStoreTrackSeeds", False)

    # Toggle track slimming
    icf.addFlag("Tracking.doSlimming", lambda prevFlags:
                not(prevFlags.Beam.Type in [BeamType.SingleBeam,
                                            BeamType.Cosmics] \
                    or prevFlags.Tracking.doHighPileup \
                    or prevFlags.Tracking.doVtxLumi) )

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
    # Turn on InDetRecStatistics
    icf.addFlag("Tracking.doStats", False)
    # Switch for track observer tool
    icf.addFlag("Tracking.doTIDE_AmbiTrackMonitoring", False)

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
