# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Flags used in CI tests

from TrkConfig.TrkConfigFlags import TrackingComponent

def actsLegacyWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: add Acts (legacy like) workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.ActsLegacyChain]

def actsWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in unit test: schedule a pure ACTS workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.doAmbiguityResolution = True
    flags.Tracking.doITkFastTracking = True
    flags.Tracking.recoChain = [TrackingComponent.ActsChain]

def actsScoreBasedAmbiguityWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in unit test: schedule a pure ACTS (legacy like) workflow to reco sequence"""
    actsLegacyWorkflowFlags(flags)
    from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy
    flags.Acts.AmbiguitySolverStrategy = AmbiguitySolverStrategy.ScoreBased
        
def actsHeavyIonFlags(flags) -> None:
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.doAmbiguityResolution = False
    flags.Tracking.recoChain = [TrackingComponent.ActsHeavyIon]


# Validation workflows

def actsValidateClustersFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use cluster conversion [xAOD -> InDet] with both Athena and Acts sequences"""
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateClusters]

def actsValidateSpacePointsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use for validating Athena-based space point formation"""
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateSpacePoints]

def actsCoreValidateSpacePointsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use for validating ACTS-based space point formation"""
    from ActsConfig.ActsConfigFlags import SpacePointStrategy
    flags.Acts.SpacePointStrategy = SpacePointStrategy.ActsCore
    actsValidateSpacePointsFlags(flags)
    
def actsValidateSeedsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction"""
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateSeeds]
    flags.Tracking.writeSeedValNtuple = True

def actsValidateConversionSeedsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction"""
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                TrackingComponent.ActsValidateConversionSeeds]
    flags.Tracking.writeSeedValNtuple = True

def actsValidateLargeRadiusSeedsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction"""
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                TrackingComponent.ActsValidateLargeRadiusSeeds]
    flags.Tracking.writeSeedValNtuple = True
    
def actsValidateOrthogonalSeedsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction (orthogonal seeding)"""
    from ActsConfig.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy = SeedingStrategy.Orthogonal
    actsValidateSeedsFlags(flags)

def actsValidateGbtsSeedsFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction (GBTS seeding)"""
    from ActsConfig.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy = SeedingStrategy.Gbts 
    actsValidateSeedsFlags(flags)

def actsValidateTracksFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use ActsTrackFinding during reconstruction"""
    flags.Acts.doAmbiguityResolution = False
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateTracks]

def actsValidateResolvedTracksFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use ActsTrackFinding during reconstruction with ambi. resolution"""
    actsValidateTracksFlags(flags)
    flags.Acts.doAmbiguityResolution = True

def actsValidateAmbiguityResolutionFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use Acts Ambiguity Resolution after Athena reconstruction"""
    flags.Reco.EnableHGTDExtension = False 
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateAmbiguityResolution]

def actsValidateGSFFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use GaussianSumFitter"""
    from ActsConfig.ActsConfigFlags import TrackFitterType
    flags.Acts.trackFitterType = TrackFitterType.GaussianSumFitter

def actsValidateGX2FFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use GlobalChiSquareFitter"""
    from ActsConfig.ActsConfigFlags import TrackFitterType
    flags.Acts.trackFitterType = TrackFitterType.GlobalChiSquareFitter

def actsGSFEgammaFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: ACTS GSF refitting for electron ACTS tracks"""
    flags.DQ.useTrigger = False
    flags.Acts.doAnalysis =  False
    flags.Acts.doMonitoring = False
    flags.Acts.doAmbiguityResolution = True
    flags.Tracking.recoChain = [ TrackingComponent.ActsLegacyChain]
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.doITkConversion = False
    flags.Acts.GsfRefitActs = True
    flags.Acts.GsfDirectNavigation = True
    
