# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class SeedingStrategy(FlagEnum):
    Default = "Default"
    Orthogonal = "Orthogonal"
    Gbts = "Gbts"

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
    actscf.addFlag('Acts.EDM.PersistifyClusters', False)
    actscf.addFlag('Acts.EDM.PersistifySpacePoints', False)
    actscf.addFlag('Acts.EDM.PersistifyTracks', False)
    actscf.addFlag('Acts.EDM.PersistifyTrackParticles',False)
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

    # Monitoring
    actscf.addFlag('Acts.doMonitoring', False)
    actscf.addFlag('Acts.doAnalysis', False)
    actscf.addFlag('Acts.storeTrackStateInfo', False)

    # SpacePoint
    actscf.addFlag("Acts.SpacePointStrategy", SpacePointStrategy.ActsTrk, type=SpacePointStrategy)  # Define SpacePoint Strategy

    # Seeding
    actscf.addFlag("Acts.SeedingStrategy", SeedingStrategy.Default, type=SeedingStrategy)  # Define Seeding Strategy

    # Track finding
    actscf.addFlag('Acts.PixelCalibrationStrategy', PixelCalibrationStrategy.AnalogueClusteringAfterSelection, type=PixelCalibrationStrategy)
    actscf.addFlag('Acts.doRotCorrection', True)
    actscf.addFlag('Acts.doPrintTrackStates', False)
    actscf.addFlag('Acts.skipDuplicateSeeds', True)
    actscf.addFlag('Acts.trackFindingTrackSelectorConfig', 1) # 0=no selection, 1=default track selection (chi2<25,25), 2=Athena chi2 cut (chi2<9,25), 3=no looser cuts in branch stopper, 4=no pix hit/pix hole/str hole cuts
    actscf.addFlag('Acts.doTwoWayCKF', True) # run CKF twice, first with forward propagation with smoothing, then with backward propagation
    actscf.addFlag('Acts.useStripSeedsFirst', False) # switch order of seed collections
    actscf.addFlag('Acts.reverseTrackFindingForStrips', False) # track finding starts going inward for strip seeds
    actscf.addFlag('Acts.useHGTDClusterInTrackFinding', False) # use HGTD cluster in track finding

    actscf.addFlag('Acts.doAmbiguityResolution', True)
    actscf.addFlag('Acts.AmbiguitySolverStrategy', AmbiguitySolverStrategy.Greedy, type=AmbiguitySolverStrategy)  # Define Ambiguity Solver Strategy

    # Track fitting
    actscf.addFlag('Acts.writeTrackCollection', False) # save to file (ESD, AOD) the Resolved and Refitted track collections
    actscf.addFlag('Acts.fitFromPRD', False) # Acts.writeTrackCollection needs to be True for either cases. If Acts.fitFromPRD is False, fit from ROT; else, fit from PRD
    actscf.addFlag('Acts.trackFitterType', TrackFitterType.KalmanFitter, type=TrackFitterType) # Define Tracking algorithm for refitting

    # GSF specific flags
    actscf.addFlag("Acts.useActsGsfInEgamma", False)
    actscf.addFlag("Acts.GsfMaxComponents", 12)
    actscf.addFlag("Acts.GsfComponentMergeMethod", 'eMaxWeight')
    actscf.addFlag("Acts.GsfDirectNavigation", False)
    actscf.addFlag("Acts.GsfOutlierChi2Cut", 20.0)

    actscf.addFlag('Acts.useDefaultActsMeasurementSelector', False) # if True, uses no outlier chi2 cut as before (chi2<25,inf)

    return actscf
