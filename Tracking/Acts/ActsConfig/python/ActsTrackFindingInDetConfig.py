
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SeedingStrategy
import AthenaCommon.SystemOfUnits as Units
from ActsInterop import UnitConstants

# Tools

def isdet(flags,
          *,
          pixel: list = None,
          strip: list = None,
          noStrip: bool = False) -> list:
    keys = []
    if flags.Detector.EnablePixel and pixel is not None:
        keys += pixel
    if flags.Detector.EnableSCT and strip is not None and not noStrip:
        keys += strip

    return keys

def seedOrder(flags,
          *,
          pixel: list = None,
          strip: list = None) -> list:
    keys = isdet(flags, pixel=pixel, strip=strip)
    if flags.Acts.useStripSeedsFirst:
        keys.reverse()
    return keys

def ActsInDetTrackStatePrinterToolCfg(flags,
                                 name: str = "ActsInDetTrackStatePrinterTool",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("InputSpacePoints", isdet(flags, noStrip=False,
                                                pixel=['PixelSpacePoints'],
                                                strip=['SCT_SpacePoints', 'OverlapSpacePoints']))

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinterTool(name, **kwargs))
    return acc

# ACTS only algorithm

def ActsInDetMainTrackFindingAlgCfg(flags,
                               name: str = "ActsTrackFindingAlg",
                               **kwargs) -> ComponentAccumulator:
    def tolist(c):
        return c if isinstance(c, list) else [c]

    acc = ComponentAccumulator()

    from ActsConfig.ActsGeometryConfig import ActsInDetVolumeIdToDetectorCollectionMappingAlgCfg

    acc.merge( ActsInDetVolumeIdToDetectorCollectionMappingAlgCfg(flags) )
    kwargs.setdefault("ActsVolumeIdToDetectorElementCollectionMapKey", "VolumeIdToDetectorElementCollectionMap")

    if flags.Detector.EnablePixel:
        from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementStatusAlgCfg
        acc.merge(PixelDetectorElementStatusAlgCfg(flags))
    if flags.Detector.EnableSCT:
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_DetectorElementStatusAlgCfg
        acc.merge(SCT_DetectorElementStatusAlgCfg(flags))
    kwargs.setdefault("DetElStatus", seedOrder(flags, pixel=["PixelDetectorElementStatus"], strip=["SCTDetectorElementStatus"]))

    # Seed labels and collections.
    # These 3 lists must match element for element, reversed if flags.Acts.useStripSeedsFirst is True.
    # Maybe it is best to start with strips where the occupancy is lower.
    kwargs.setdefault("SeedLabels", seedOrder(flags, pixel=["PPP"], strip=["SSS"]))
    kwargs.setdefault("SeedContainerKeys", seedOrder(flags, pixel=["ActsPixelSeeds"], strip=["ActsSCT_Seeds"]))
    kwargs.setdefault('DetectorElementsKeys', seedOrder(flags, pixel=['PixelDetectorElementCollection'], strip=['SCT_DetectorElementCollection']))
    if flags.Acts.Tracks.doAnalysis:
        kwargs.setdefault("SeedDestiny", [f'{seedkey}Destiny' for seedkey in kwargs["SeedContainerKeys"]])

    kwargs.setdefault("UncalibratedMeasurementContainerKeys", isdet(flags, pixel=["PixelClusters"], strip=["SCT_Clusters"]))

    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks')

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", True)# flags.Acts.skipDuplicateSeeds
    kwargs.setdefault("seedMeasOffset", 1)

    # Ambi strategy 0 means do the ambiguity resolution outside the track finding.
    kwargs.setdefault("ambiStrategy", flags.Acts.AmbiguitySolverMode.value)
    
    if (not flags.Acts.doAmbiguityResolution) :
        kwargs.setdefault("MaximumSharedHits", 3)
        kwargs.setdefault("MaximumIterations", 10000)
        kwargs.setdefault("NMeasurementsMin", 7)
    
    kwargs.setdefault("refitSeeds", seedOrder(flags, pixel=[False], strip=[False]))
    kwargs.setdefault("doTwoWay", flags.Acts.doTwoWayCKF)
    kwargs.setdefault("inflateCovarianceTwoWay", True)
    kwargs.setdefault("twoWayinflateCovarianceFactor", 100.0)
    kwargs.setdefault("autoReverseSearch", flags.Tracking.ActiveConfig.autoReverseSearch)
    # kwargs.setdefault("useTopSpRZboundary",[ 229.0 * UnitConstants.mm, 450.0 * UnitConstants.mm])
    
    # forceTrackOnSeed isn't effective with secondary passes, which will have removed most/all of the seed measurements from the measurement containers.
    kwargs.setdefault("forceTrackOnSeed", flags.Acts.forceTrackOnSeed and not flags.Tracking.ActiveConfig.isSecondaryPass)

    # Borrow many settings from flags.Tracking.ActiveConfig, normally initialised in createITkTrackingPassFlags() at
    # https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkConfig/python/TrackingPassFlags.py#L121

    # bins in |eta|, used for both MeasurementSelectorConfig and TrackSelector::EtaBinnedConfig
    kwargs.setdefault("etaBins", flags.Tracking.ActiveConfig.etaBins) # 
    # new default chi2 cuts optimise efficiency vs speed. Set same value as Athena's Xi2maxNoAdd.
    kwargs.setdefault("chi2CutOff", tolist(flags.Tracking.ActiveConfig.Xi2max*1))
    kwargs.setdefault("chi2OutlierCutOff", tolist(flags.Tracking.ActiveConfig.Xi2maxNoAdd*1))
    kwargs.setdefault("checkCounts", True)
    kwargs.setdefault("doBranchStopper", True)

    kwargs.setdefault("branchStopperPtMinFactor", 0.9)
    kwargs.setdefault("branchStopperAbsEtaMaxExtra", 0.1)

    # Loosen the requirement on the minimum number of measurements on track candidate
    # during track finding for tracks above a certain eta
    kwargs.setdefault("branchStopperMeasCutReduce", flags.Acts.branchStopperMeasCutReduce)
    kwargs.setdefault("branchStopperAbsEtaMeasCut", flags.Acts.branchStopperAbsEtaMeasCut)

    kwargs.setdefault("numMeasurementsCutOff", [1])

    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized by abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins", [eta/10. for eta in range(5, 40, 5)]) # eta 0.0 - 4.0 in steps of 0.5

    kwargs.setdefault("absEtaMax", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("ptMin", [p / Units.GeV * UnitConstants.GeV for p in tolist(flags.Tracking.ActiveConfig.minPT)])
    # z0 cut is the same for all eta bins. I use the size of the eta bins limits minus one to find the number of bins.
    kwargs.setdefault("z0Min", [-flags.Tracking.ActiveConfig.maxZImpact / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("z0Max", [ flags.Tracking.ActiveConfig.maxZImpact / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("d0Min", [-d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpactList)])
    kwargs.setdefault("d0Max", [ d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpactList)])
    kwargs.setdefault("minMeasurements", tolist([ 8, 8, 8]))
    kwargs.setdefault("maxHoles", tolist(flags.Tracking.ActiveConfig.maxHoles))
    kwargs.setdefault("minPixelHits", tolist(flags.Tracking.ActiveConfig.minPixel))
    kwargs.setdefault("maxPixelHoles", tolist(flags.Tracking.ActiveConfig.maxPixelHoles))
    kwargs.setdefault("maxStripHoles", tolist(flags.Tracking.ActiveConfig.maxSctHoles))
    # The shared hits are not calculated until *after* the track selection, so maxSharedHits is not used.
    # Even if that were not the case, we need the ambiguity solver to decide which track to drop.
    ### kwargs.setdefault("maxSharedHits", tolist(flags.Tracking.ActiveConfig.maxShared))

    # GBTS produces much purer seeds, so the branch stopper selections aren't needed with GBTS seeds.
    if flags.Acts.SeedingStrategy is not SeedingStrategy.Gbts:
        kwargs.setdefault("ptMinMeasurements", seedOrder(flags, pixel=[3], strip=[6]))
        kwargs.setdefault("absEtaMaxMeasurements", seedOrder(flags, pixel=[3], strip=[999999]))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg, ActsGeometryRealmConvTool
    kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    kwargs.setdefault("GeometryRealmConvTool", acc.getPrimaryAndMerge(ActsGeometryRealmConvTool(flags)))

    if 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
        kwargs.setdefault('TrackParamsEstimationTool', acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags)))
        
    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )
        
    if flags.Acts.doPrintTrackStates and 'TrackStatePrinter' not in kwargs:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsInDetTrackStatePrinterToolCfg(flags)),
        )

    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg 
        kwargs.setdefault(
            'FitterTool',
            acc.popToolsAndMerge(ActsFitterCfg(flags, 
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=float('inf')))
        )

    # !!! Calibrator is not used for Inner Detector yet
    if 'PixelCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType

        if flags.Beam.Type is not BeamType.Cosmics:
            from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy
            from ActsConfig.ActsMeasurementCalibrationConfig import ActsAnalogueClusteringToolCfg
            
            if flags.Acts.PixelCalibrationStrategy in (PixelCalibrationStrategy.AnalogueClustering,
                                                       PixelCalibrationStrategy.AnalogueClusteringAfterSelection,
                                                       PixelCalibrationStrategy.NNClustering):

                kwargs.setdefault(
                    'PixelCalibrator',
                    acc.popToolsAndMerge(ActsAnalogueClusteringToolCfg(flags,
                                                                       CalibrateAfterMeasurementSelection = flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClusteringAfterSelection))
                )
    # !!! Calibrator is not used for Inner Detector yet
    if 'StripCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType
        if flags.Beam.Type is not BeamType.Cosmics:
            from ActsConfig.ActsMeasurementCalibrationConfig import ActsStripCalibrationToolCfg
            from ActsConfig.ActsConfigFlags import StripCalibrationStrategy

            if flags.Acts.StripCalibrationStrategy in (StripCalibrationStrategy.DigitalCalibration,
                                                       StripCalibrationStrategy.DigitalCalibrationAfterSelection) :

                kwargs.setdefault(
                    'StripCalibrator',
                    acc.popToolsAndMerge(ActsStripCalibrationToolCfg(flags,
                                                                     CalibrateAfterMeasurementSelection = flags.Acts.StripCalibrationStrategy is StripCalibrationStrategy.DigitalCalibrationAfterSelection))
                )

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc



def ActsInDetTrackFindingCfg(flags,
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Define Uncalibrated Measurement keys
    dataPrepPrefix = f'{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}'
    if not flags.Tracking.ActiveConfig.isSecondaryPass:
        dataPrepPrefix = ''
    pixelClusters = f'{dataPrepPrefix}PixelClusters'
    stripClusters = f'{dataPrepPrefix}SCT_Clusters'    
    # If cache is activated the keys have "_Cached" as postfix
    if flags.Acts.useCache:
        pixelClusters += '_Cached'
        stripClusters += '_Cached'
    # Consider case detectors are not active

    # Understand what are the seeds we need to consider
    pixelSeedLabels = ['PPP']
    stripSeedLabels = ['SSS']

    # Consider if pixel seeds and SCT seeds are needed, for the moment, both are kept for Inner Detector

    # Now set the seed and estimated parameters keys accordingly
    pixelSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}PixelSeeds']
    stripSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}SCT_Seeds']
    pixelDetElements = ['PixelDetectorElementCollection']
    stripDetElements = ['SCT_DetectorElementCollection']
    if pixelSeedLabels is None:
        pixelSeedKeys = None
        pixelDetElements = None
    if stripSeedLabels is None:
        stripSeedKeys = None
        stripDetElements = None

    kwargs.setdefault('ACTSTracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('UncalibratedMeasurementContainerKeys', isdet(flags, pixel=[pixelClusters], strip=[stripClusters]))
    kwargs.setdefault('SeedLabels', seedOrder(flags, pixel=pixelSeedLabels, strip=stripSeedLabels))
    kwargs.setdefault('SeedContainerKeys', seedOrder(flags, pixel=pixelSeedKeys, strip=stripSeedKeys))
    kwargs.setdefault('DetectorElementsKeys', seedOrder(flags, pixel=pixelDetElements, strip=stripDetElements))

    acc.merge(ActsInDetMainTrackFindingAlgCfg(flags,
                                         name=f"{flags.Tracking.ActiveConfig.extension}TrackFindingAlg",
                                         **kwargs))

    # # Analysis extensions
    # if flags.Acts.Tracks.doAnalysis:
    #     from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
    #     acc.merge(ActsTrackAnalysisAlgCfg(flags,
    #                                       name=f"{flags.Tracking.ActiveConfig.extension}TrackAnalysisAlg",
    #                                       TracksLocation=f"{flags.Tracking.ActiveConfig.extension}Tracks"))

    #     # Seed To Track Monitoring
    #     if len(kwargs["SeedContainerKeys"]) != len(kwargs["DetectorElementsKeys"]):
    #         raise AttributeError("SeedContainerKeys and DetectorElementsKeys must have same size")

    #     for i in range(0, len(kwargs["SeedContainerKeys"])):
    #         seedKey = kwargs["SeedContainerKeys"][i]
    #         detElKey = kwargs["DetectorElementsKeys"][i]

    #         # make seed params
    #         from ActsConfig.ActsAnalysisConfig import ActsBaseSeedsToTrackParamsAlgCfg
    #         acc.merge(ActsBaseSeedsToTrackParamsAlgCfg(flags,
    #                                                    name = f'{seedKey}SeedsToTrackParamsAlg',
    #                                                    InputSeedContainerKey = seedKey,
    #                                                    DetectorElementsKey = detElKey,
    #                                                    OutputTrackParamsCollectionKey = f'{seedKey}Params'))

    #         from ActsConfig.ActsAnalysisConfig import ActsSeedToTrackAnalysisAlgCfg
    #         acc.merge(ActsSeedToTrackAnalysisAlgCfg(flags,
    #                                                 name = f'{seedKey}ToTrackAnalysisAlg',
    #                                                 InputSeedCollection = seedKey,
    #                                                 InputTrackParamsCollection = f'{seedKey}Params',
    #                                                 InputDestinyCollection = f'{seedKey}Destiny'))

    # Persistification
    if flags.Acts.EDM.PersistifyTracks:
        trackColl = kwargs['ACTSTracksLocation']
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        acc.merge(ActsToXAODTrackConverterAlgCfg(flags,
                                                 name = f'{trackColl}ToXAODConverterAlg',
                                                 InputActsTracksLocation = trackColl,
                                                 OutputActsTracksLocation = trackColl))
        
        toAOD = []
        prefix = f"{flags.Tracking.ActiveConfig.extension}"
        toAOD += [f"xAOD::TrackSummaryContainer#{prefix}TrackSummary",
                  f"xAOD::TrackSummaryAuxContainer#{prefix}TrackSummaryAux.",
                  f"xAOD::TrackStateContainer#{prefix}TrackStates",
                  f"xAOD::TrackStateAuxContainer#{prefix}TrackStatesAux.-uncalibratedMeasurement",
                  f"xAOD::TrackParametersContainer#{prefix}TrackParameters",
                  f"xAOD::TrackParametersAuxContainer#{prefix}TrackParametersAux.",
                  f"xAOD::TrackJacobianContainer#{prefix}TrackJacobians",
                  f"xAOD::TrackJacobianAuxContainer#{prefix}TrackJacobiansAux.",
                  f"xAOD::TrackMeasurementContainer#{prefix}TrackMeasurements",
                  f"xAOD::TrackMeasurementAuxContainer#{prefix}TrackMeasurementsAux.",
                  f"xAOD::TrackSurfaceContainer#{prefix}TrackStateSurfaces",
                  f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackStateSurfacesAux.",
                  f"xAOD::TrackSurfaceContainer#{prefix}TrackSurfaces",
                  f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackSurfacesAux."]
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))
        
    return acc


def ActsInDetMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsInDetScoreBasedAmbiguityResolutionAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ActsResolvedTracks')
    kwargs.setdefault('MinScore',1.0)
    kwargs.setdefault('MinScoreSharedTracks', 1.0)
    kwargs.setdefault('MaxSharedTracksPerMeasurement', 20)
    kwargs.setdefault('MaxShared', 5)
    kwargs.setdefault('MinUnshared', 5)
    kwargs.setdefault('UseAmbiguityScoring', True)
    kwargs.setdefault('jsonFileName', 'ActsAmbiguityConfig.json')

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.ScoreBasedAmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsInDetMainAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsInDetAmbiguityResolutionAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ActsResolvedTracks')
    kwargs.setdefault('MaximumSharedHits', 3)
    kwargs.setdefault('MaximumIterations', 10000)
    kwargs.setdefault('NMeasurementsMin', 7)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsInDetAmbiguityResolutionCfg(flags,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('ResolvedTracksLocation', f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks")
    from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy
            
    if flags.Acts.AmbiguitySolverStrategy is AmbiguitySolverStrategy.ScoreBased:
        acc.merge(ActsInDetMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                                    name=f"{flags.Tracking.ActiveConfig.extension}ScoreBasedAmbiguityResolutionAlg",
                                                    **kwargs))
    else:
        acc.merge(ActsInDetMainAmbiguityResolutionAlgCfg(flags,
                                                    name=f"{flags.Tracking.ActiveConfig.extension}AmbiguityResolutionAlg",
                                                    **kwargs))
    # Analysis extensions
    if flags.Acts.Tracks.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
        acc.merge(ActsTrackAnalysisAlgCfg(flags,
                                          name=f"{flags.Tracking.ActiveConfig.extension}ResolvedTrackAnalysisAlg",
                                          TracksLocation=f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"))

    # Persistification
    if flags.Acts.EDM.PersistifyTracks:
        trackColl = kwargs['ResolvedTracksLocation']
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        acc.merge(ActsToXAODTrackConverterAlgCfg(flags,
                                                 name = f'{trackColl}ToXAODConverterAlg',
                                                 InputActsTracksLocation = trackColl,
                                                 OutputActsTracksLocation = trackColl))
        
        toAOD = []
        prefix = f"{flags.Tracking.ActiveConfig.extension}Resolved"
        toAOD += [f"xAOD::TrackSummaryContainer#{prefix}TrackSummary",
                  f"xAOD::TrackSummaryAuxContainer#{prefix}TrackSummaryAux.",
                  f"xAOD::TrackStateContainer#{prefix}TrackStates",
                  f"xAOD::TrackStateAuxContainer#{prefix}TrackStatesAux.-uncalibratedMeasurement",
                  f"xAOD::TrackParametersContainer#{prefix}TrackParameters",
                  f"xAOD::TrackParametersAuxContainer#{prefix}TrackParametersAux.",
                  f"xAOD::TrackJacobianContainer#{prefix}TrackJacobians",
                  f"xAOD::TrackJacobianAuxContainer#{prefix}TrackJacobiansAux.",
                  f"xAOD::TrackMeasurementContainer#{prefix}TrackMeasurements",
                  f"xAOD::TrackMeasurementAuxContainer#{prefix}TrackMeasurementsAux.",
                  f"xAOD::TrackSurfaceContainer#{prefix}TrackStateSurfaces",
                  f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackStateSurfacesAux.",
                  f"xAOD::TrackSurfaceContainer#{prefix}TrackSurfaces",
                  f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackSurfacesAux."]        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
        acc.merge(addToAOD(flags, toAOD))

    return acc
