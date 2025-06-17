# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class SeedingStrategy(FlagEnum):
    Default = "Default"
    Orthogonal = "Orthogonal"
    Gbts = "Gbts"
    Gbts2 = "Gbts2"

class AmbiguitySolverStrategy(FlagEnum):
    Greedy = "GreedySolver"
    ScoreBased = "ScoreBasedAmbiguitySolver"


# This is temporary during the integration of ACTS.
class SpacePointStrategy(FlagEnum):
    ActsCore = "ActsCore" # ACTS-based SP formation
    ActsTrk = "ActsTrk" #SP formation without ACTS

class TrackFitterType(FlagEnum):
    KalmanFitter = 'KalmanFitter' # default ACTS fitter to choose
    GaussianSumFitter = 'GaussianSumFitter' # new experimental implementation
    GlobalChiSquareFitter = 'GlobalChiSquareFitter' # new experimental implementation

# Flag for pixel calibration strategy during track finding
# - use cluster as is (Uncalibrated)
# - perform AnalogueClustering either before selecting
#   measurements for extending tracks (AnalogueClustering)
# - or only apply the AnalogueClustering to selected measurements
#   (AnalogueClusteringAfterSelection)
class PixelCalibrationStrategy(FlagEnum):
    Uncalibrated = "Uncalibrated"
    AnalogueClustering = "AnalogueClustering"
    AnalogueClusteringAfterSelection = "AnalogueClusteringAfterSelection"

def createActsConfigFlags():
    actscf = AthConfigFlags()
    
    # General Flags
    actscf.addFlag('Acts.EDM.PersistifyClusters', lambda pcf: pcf.Acts.EDM.PersistifySpacePoints)
    actscf.addFlag('Acts.EDM.PersistifySpacePoints', False)
    actscf.addFlag('Acts.EDM.PersistifyTracks', False)
    actscf.addFlag('Acts.useCache', False)
    
    # Scheduling
    actscf.addFlag('Acts.doITkConversion', False)
    actscf.addFlag('Acts.doLargeRadius', False)
    actscf.addFlag('Acts.doLowPt', False)
    
    # Geometry Flags

    # MaterialSource can be:
    # a path to a local JSON file
    # 'Default' : material map source is evaluated from the geometry tag
    # 'None'    : no material map is provided
    actscf.addFlag('Acts.TrackingGeometry.MaterialSource', 'Default')
    actscf.addFlag('Acts.TrackingGeometry.MaterialCalibrationFolder', 'ACTS/MaterialMaps/ITk')
    actscf.addFlag('Acts.TrackingGeometry.MaterialFileExtension', '')
    actscf.addFlag('Acts.TrackingGeometry.UseBlueprint', False)

    ## Enable Tracking geometry with additional passive layers
    actscf.addFlag('Acts.TrackingGeometry.InsertITkPassiveMaterialLayers', False)
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerRadii', [70.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerHalflengthZ', [240.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerThickness', [1.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerRadii', [195., 260.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerHalflengthZ', [370., 370.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerThickness', [1., 1.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkStripBarrelLayerRadii', [480., 665., 880.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkStripBarrelLayerHalflengthZ', [1370., 1370., 1370.])
    actscf.addFlag('Acts.TrackingGeometry.PassiveITkStripBarrelLayerThickness', [1., 1., 1.])

    # Monitoring
    actscf.addFlag('Acts.doMonitoring', False)
    actscf.addFlag('Acts.doAnalysis', False)
    actscf.addFlag('Acts.doAnalysisNtuples', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.Clusters.doAnalysis', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.SpacePoints.doAnalysis', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.Seeds.doAnalysis', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.Tracks.doAnalysis', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.Particles.doAnalysis', lambda pcf: pcf.Acts.doAnalysis)
    actscf.addFlag('Acts.storeTrackStateInfo', False)

    # Cluster
    actscf.addFlag("Acts.Clusters.UseWeightedPosition", False)
    
    # SpacePoint
    actscf.addFlag("Acts.SpacePointStrategy", SpacePointStrategy.ActsTrk, type=SpacePointStrategy)  # Define SpacePoint Strategy

    # Seeding
    actscf.addFlag("Acts.SeedingStrategy", SeedingStrategy.Default, type=SeedingStrategy)  # Define Seeding Strategy

    # Track finding
    actscf.addFlag('Acts.PixelCalibrationStrategy', PixelCalibrationStrategy.AnalogueClusteringAfterSelection, type=PixelCalibrationStrategy)
    actscf.addFlag('Acts.doRotCorrection', True)
    actscf.addFlag('Acts.doPrintTrackStates', False)
    actscf.addFlag('Acts.skipDuplicateSeeds', True)
    actscf.addFlag('Acts.doTwoWayCKF', True) # run CKF twice, first with forward propagation with smoothing, then with backward propagation
    actscf.addFlag('Acts.useStripSeedsFirst', False) # switch order of seed collections
    actscf.addFlag('Acts.autoReverseSearchCKF', False) # track finding starts going inward first if we are outside the defined RZ boundary
    actscf.addFlag('Acts.useHGTDClusterInTrackFinding', False) # use HGTD cluster in track finding
    actscf.addFlag('Acts.branchStopperMeasCutReduce', 2)
    actscf.addFlag('Acts.branchStopperAbsEtaMeasCut', 1.2)
        
    # Ambiguity resolution

    actscf.addFlag('Acts.doAmbiguityResolution', True)
    actscf.addFlag('Acts.AmbiguitySolverStrategy', AmbiguitySolverStrategy.Greedy, type=AmbiguitySolverStrategy)  # Define Ambiguity Solver Strategy

    # Calibrations
    actscf.addFlag('Acts.OnTrackCalibration.performCovarianceCalibration', True) # perform calibration of covariance during on track analogue cluster calibration
    
    # Track fitting
    actscf.addFlag('Acts.writeTrackCollection', False) # save to file (ESD, AOD) the Resolved and Refitted track collections
    actscf.addFlag('Acts.fitFromPRD', False) # Acts.writeTrackCollection needs to be True for either cases. If Acts.fitFromPRD is False, fit from ROT; else, fit from PRD
    actscf.addFlag('Acts.trackFitterType', TrackFitterType.KalmanFitter, type=TrackFitterType) # Define Tracking algorithm for refitting

    # GSF specific flags
    actscf.addFlag("Acts.GsfRefitLegacyTrk", False) # Refit Legacy tracks using ACTS GSF
    actscf.addFlag("Acts.GsfRefitActs", False) # Refit ACTS tracks using ACTS GSF
    actscf.addFlag("Acts.GsfMaxComponents", 12)
    actscf.addFlag("Acts.GsfComponentMergeMethod", 'MaxWeight')
    actscf.addFlag("Acts.GsfDirectNavigation", False)
    actscf.addFlag("Acts.GsfOutlierChi2Cut", 1e4) # Effectively no cut. Compatible with legacy

    # Decorations
    actscf.addFlag('Acts.decoratePRD.sdoSiHit', lambda pcf: pcf.Tracking.doTIDE_AmbiTrackMonitoring)
    
    return actscf
