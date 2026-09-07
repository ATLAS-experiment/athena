# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Flags used in CI tests

from TrkConfig.TrkConfigFlags import TrackingComponent
from AthenaConfiguration.Enums import LHCPeriod

# actsProductionFlags are default now
def actsProductionFlags(flags) -> None:
    """flags for ACTS reconstruction to be used for production jobs"""
    # Reco chain to ACTS flavour
    flags.Tracking.recoChain = [TrackingComponent.ActsChain]
    # Save Trk::Track link for combined muon reconstruction
    flags.Acts.doxAODToTrkConversion = True
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

def athenaLegacyTrackingFlags(flags) -> None:
    """flags to revert to Athena legacy Run 4 tracking reconstruction, kept for testing purposes alone"""
    # Reco chain to ACTS flavour
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain]
    flags.Tracking.doITkFastTracking = False
    flags.Tracking.doPixelDigitalClustering = False
    flags.HGTD.doActs = False

def actsLegacyWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: add Acts (legacy like) workflow to reco sequence"""
    flags.Tracking.recoChain = [TrackingComponent.ActsLegacyChain]
    flags.Tracking.doITkFastTracking = False

def actsInnerDetectorWorkflowFlags(flags) -> None:
    """flags for Reco_tf with CA used in unit test: schedule a pure ACTS workflow to reco sequence, with Inner Detector settings"""
    flags.Tracking.recoChain = [TrackingComponent.ActsChain]
        
def actsHeavyIonFlags(flags) -> None:
    flags.Tracking.recoChain = [TrackingComponent.ActsHeavyIon]
    flags.Tracking.doITkFastTracking = False


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

def actsValidateF100Flags(flags) -> None:
    actsProductionFlags(flags)
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateF100]

def actsValidateF150Flags(flags) -> None:
    actsValidateF100Flags(flags)
    flags.Tracking.recoChain = [TrackingComponent.ActsValidateF150]
