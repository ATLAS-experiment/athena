# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Flags used in CI tests

from TrkConfig.TrkConfigFlags import TrackingComponent
from AthenaConfiguration.Enums import LHCPeriod


def actsProductionFlags(flags) -> None:
    """flags for ACTS reconstruction to be used for production jobs"""
    # Reco chain to ACTS flavour
    flags.Tracking.recoChain = [TrackingComponent.ActsChain]
    # Track reconstruction algorithms
    flags.Acts.doAmbiguityResolution = False
    flags.Tracking.doITkFastTracking = True
    # Configurations
    # - calibration strategy is set centrally
    # - seeding strategy set by the user: default is GridTriplet
    flags.Tracking.doPixelDigitalClustering = lambda pcf: pcf.GeoModel.Run >= LHCPeriod.Run4    
    # e-gamma components
    flags.Acts.GsfRefitActs = True
    flags.Acts.GsfDirectNavigation = True
    # HGTD components
    flags.HGTD.doActs = True

def actsLegacyWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: add Acts (legacy like) workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.GsfRefitActs = True
    flags.Acts.GsfDirectNavigation=True
    flags.Tracking.recoChain = [TrackingComponent.ActsLegacyChain]
    flags.Tracking.doPixelDigitalClustering = lambda pcf: pcf.GeoModel.Run >= LHCPeriod.Run4

def actsInnerDetectorWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in unit test: schedule a pure ACTS workflow to reco sequence, with Inner Detector settings"""
    flags.Tracking.recoChain = [TrackingComponent.ActsChain]

def actsScoreBasedAmbiguityWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in unit test: schedule a pure ACTS (legacy like) workflow to reco sequence"""
    actsLegacyWorkflowFlags(flags)
    from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy
    flags.Acts.AmbiguitySolverStrategy = AmbiguitySolverStrategy.ScoreBased
        
def actsHeavyIonFlags(flags) -> None:
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.doAmbiguityResolution = False
    flags.Tracking.doPixelDigitalClustering = lambda pcf: pcf.GeoModel.Run >= LHCPeriod.Run4
    flags.Tracking.recoChain = [TrackingComponent.ActsHeavyIon]


# Validation workflows
    
    
def actsValidateLargeRadiusStandaloneFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use legacy primary pass and Acts LRT pass"""
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                TrackingComponent.ActsValidateLargeRadiusStandalone]
    flags.Tracking.writeSeedValNtuple = True

def actsValidateClustersFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use cluster conversion [xAOD -> InDet] with both Athena and Acts sequences"""
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateClusters]

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

def actsValidateF100Flags(flags) -> None:
    actsProductionFlags(flags)
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateF100]

def actsValidateF150Flags(flags) -> None:
    actsValidateF100Flags(flags)
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateF150]
    from ActsConfig.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy = SeedingStrategy.F150
