# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class SeedingStrategy(FlagEnum):
    Default = "Default"
    Orthogonal = "Orthogonal"
    Gbts = "Gbts"
    Gbts2 = "Gbts2"
    GridTriplet = "GridTriplet"
    F150 = "F150"

class AmbiguitySolverStrategy(FlagEnum):
    Greedy = "GreedySolver"
    ScoreBased = "ScoreBasedAmbiguitySolver"

# Define the Ambiguity resolution strategy modes
# OUTSIDE_TF : run the ambiguity resolution in a separate algorithm
# END_OF_TF  : run the ambiguity resolution at the end of the track finding, on the track candidate container
# DURING_TF  : remove the tracks that share too many hits during track finding, when deciding good candidates
class AmbiguitySolverMode(FlagEnum):
    OUTSIDE_TF = 0
    END_OF_TF = 1
    DURING_TF = 2

# Define the Pixel cluster calibration error strategy modes (the errors that will be used for filtering)
# CALIBRATED : load the calibration constants from database
# PITCH  : if we used broad clusters during clustering (so the error is the cluster width) rescale it by the number of pixels
#          to assign the pitch as error to the cluster
class PixelErrorStrategy(FlagEnum):
    CALIBRATED = 0
    PITCH = 1

# Define the Strip clustering errors assigned to the measurements during clustering
# These errors will be used during track finding
# PITCH : assign the pitch as cluster error
# WIDTH : use the cluster width as cluster error
# TUNED : use an error 1.05 x pitch for 1strip clusters, a 0.27 x width for 2strip clusters and width for >=3strip clusters 
class StripClusteringErrorMode(FlagEnum):
    PITCH = 0
    WIDTH = 1
    TUNED = 2

# Define the Strip cluster calibration error strategy modes (the errors that will be used for filtering)
# CLUSTERING : Use directly the clustering errors 
# PITCH  : if we used broad clusters during clustering (so the error is the cluster width) rescale it by the number of pixels
#          to assign the pitch as error to the cluster
class StripErrorStrategy(FlagEnum):
    CLUSTERING = 0
    PITCH = 1


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

# Flag for strip calibration strategy during track finding
# - use cluster as is (Uncalibrated)
# - use strip pitch / sqrt(12) as error either before selecting
#   measurements for extending tracks (DigitalCalibration)
# - or only apply it to selected measurements (DigitalCalibrationAfterSelection)
class StripCalibrationStrategy(FlagEnum):
    Uncalibrated = "Uncalibrated"
    DigitalCalibration = "DigitalCalibration"
    DigitalCalibrationAfterSelection = "DigitalCalibrationAfterSelection"
    
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
    actscf.addFlag('Acts.TrackingGeometry.ObjDebugOutput', False)

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
    actscf.addFlag("Acts.Clusters.RetrieveChargeInformation", lambda pcf: not pcf.Tracking.doPixelDigitalClustering)
    actscf.addFlag("Acts.Clusters.StripClusteringErrorMode", StripClusteringErrorMode.PITCH,type=StripClusteringErrorMode)
    actscf.addFlag("Acts.Clusters.UsePixelBroadErrors", False)
    
    # SpacePoint
    actscf.addFlag("Acts.SpacePointStrategy", SpacePointStrategy.ActsTrk, type=SpacePointStrategy)  # Define SpacePoint Strategy

    # Seeding
    actscf.addFlag("Acts.SeedingStrategy", SeedingStrategy.GridTriplet, type=SeedingStrategy)  # Define Seeding Strategy
    
    # Track finding
    actscf.addFlag('Acts.PixelCalibrationStrategy', PixelCalibrationStrategy.Uncalibrated, type=PixelCalibrationStrategy)
    actscf.addFlag('Acts.StripCalibrationStrategy', StripCalibrationStrategy.Uncalibrated, type=StripCalibrationStrategy)
    actscf.addFlag('Acts.doRotCorrection', True)
    actscf.addFlag('Acts.doPrintTrackStates', False)
    actscf.addFlag('Acts.skipDuplicateSeeds', True)
    actscf.addFlag('Acts.doTwoWayCKF', True) # run CKF twice, first with forward propagation with smoothing, then with backward propagation
    actscf.addFlag('Acts.useStripSeedsFirst', False) # switch order of seed collections
    actscf.addFlag('Acts.autoReverseSearchCKF', False) # track finding starts going inward first if we are outside the defined RZ boundary
    actscf.addFlag('Acts.useHGTDClusterInTrackFinding', False) # use HGTD cluster in track finding
    actscf.addFlag('Acts.branchStopperMeasCutReduce', 2)
    actscf.addFlag('Acts.branchStopperAbsEtaMeasCut', 1.2)
    actscf.addFlag('Acts.forceTrackOnSeed', lambda pcf: not(pcf.Acts.SeedingStrategy is SeedingStrategy.Gbts2 and
                                                            pcf.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClusteringAfterSelection)) # forceTrackOnSeed does not seem to work with GBTS seeds and analogue cluster calibration
        
    # Ambiguity resolution    
    actscf.addFlag('Acts.doAmbiguityResolution', True)
    actscf.addFlag('Acts.AmbiguitySolverStrategy', AmbiguitySolverStrategy.Greedy, type=AmbiguitySolverStrategy)  # Define Ambiguity Solver Strategy
    actscf.addFlag('Acts.AmbiguitySolverMode', lambda pcf: AmbiguitySolverMode.OUTSIDE_TF if pcf.Acts.doAmbiguityResolution else AmbiguitySolverMode.DURING_TF, type=AmbiguitySolverMode)
    
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
