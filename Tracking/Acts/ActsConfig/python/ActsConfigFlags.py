# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class SeedingStrategy(FlagEnum):
    Default = "Default"
    Orthogonal = "Orthogonal"

# This is temporary during the integration of ACTS.
class SpacePointStrategy(FlagEnum):
    ActsCore = "ActsCore" # ACTS-based SP formation
    ActsTrk = "ActsTrk" #SP formation without ACTS

class TrackFitterType(FlagEnum):
    KalmanFitter = 'KalmanFitter' # default ACTS fitter to choose
    GaussianSumFitter = 'GaussianSumFitter' # new experimental implementation

class PixelCalibrationStrategy(FlagEnum):
    Uncalibrated = "Uncalibrated"
    AnalogueClustering = "AnalogueClustering"

def createActsConfigFlags():
    actscf = AthConfigFlags()
    
    # General Flags
    actscf.addFlag('Acts.EDM.PersistifyClusters', False)
    actscf.addFlag('Acts.EDM.PersistifySpacePoints', False)
    actscf.addFlag('Acts.EDM.PersistifyTracks', False)
    actscf.addFlag('Acts.useCache', False)
    
    # Scheduling
    actscf.addFlag('Acts.doITkConversion', False)
    
    # Geometry Flags

    # MaterialSource can be:
    # a path to a local JSON file
    # 'Default' : material map source is evaluated from the geometry tag
    # 'None'    : no material map is provided
    actscf.addFlag('Acts.TrackingGeometry.MaterialSource', 'Default')
    actscf.addFlag('Acts.TrackingGeometry.MaterialCalibrationFolder', 'ACTS/MaterialMaps/ITk')

    # Monitoring
    actscf.addFlag('Acts.doMonitoring', False)
    actscf.addFlag('Acts.doAnalysis', False)

    # SpacePoint
    actscf.addFlag("Acts.SpacePointStrategy", SpacePointStrategy.ActsTrk, type=SpacePointStrategy)  # Define SpacePoint Strategy

    # Seeding
    actscf.addFlag("Acts.SeedingStrategy", SeedingStrategy.Default, type=SeedingStrategy)  # Define Seeding Strategy

    # Track finding
    actscf.addFlag('Acts.PixelCalibrationStrategy', PixelCalibrationStrategy.Uncalibrated, type=PixelCalibrationStrategy)
    actscf.addFlag('Acts.doRotCorrection', True)
    actscf.addFlag('Acts.doPrintTrackStates', False)
    actscf.addFlag('Acts.skipDuplicateSeeds', True)
    actscf.addFlag('Acts.trackFindingChi2CutOff', 15.0)
    actscf.addFlag('Acts.doTrackFindingTrackSelector', 1) # 0=no selection, 1=default track selection, 2=no eta-dependant cuts, 3=no holes/shared cuts, 4=no branch stopper
    actscf.addFlag('Acts.trackFindingMaxHoles', []) # add eta-dependent cut for branch stopper
    actscf.addFlag('Acts.doTwoWayCKF', True) # run CKF twice, first with forward propagation with smoothing, then with backward propagation

    actscf.addFlag('Acts.doAmbiguityResolution', True)

    # Track fitting
    actscf.addFlag('Acts.writeTrackCollection', False) # save to file (ESD, AOD) the Resolved and Refitted track collections
    actscf.addFlag('Acts.fitFromPRD', False) # Acts.writeTrackCollection needs to be True for either cases. If Acts.fitFromPRD is False, fit from ROT; else, fit from PRD
    actscf.addFlag('Acts.trackFitterType', TrackFitterType.KalmanFitter, type=TrackFitterType) # Define Tracking algorithm for refitting

    actscf.addFlag("Acts.useActsGsfInEgamma", False)

    return actscf
