
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from ActsInterop import UnitConstants

# Tools

def isdet(flags,
          *,
          pixel: list = None,
          strip: list = None,
          hgtd: list = None,
          noStrip: bool = False) -> list:
    keys = []
    if flags.Detector.EnableITkPixel and pixel is not None:
        keys += pixel
    if flags.Detector.EnableITkStrip and strip is not None and not noStrip:
        keys += strip
    if flags.Acts.useHGTDClusterInTrackFinding and hgtd is not None:
        keys += hgtd
    return keys

def seedOrder(flags,
          *,
          pixel: list = None,
          strip: list = None) -> list:
    keys = isdet(flags, pixel=pixel, strip=strip)
    if flags.Acts.useStripSeedsFirst:
        keys.reverse()
    return keys

def ActsTrackStatePrinterToolCfg(flags,
                                 name: str = "ActsTrackStatePrinterTool",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    kwargs.setdefault("InputSpacePoints", isdet(flags, noStrip=isFastPrimaryPass(flags),
                                                pixel=['ITkPixelSpacePoints_Cached'] if flags.Acts.useCache else ['ITkPixelSpacePoints'],
                                                strip=['ITkStripSpacePoints_Cached', 'ITkStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints']))

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinterTool(name, **kwargs))
    return acc

# ACTS only algorithm

def ActsMainTrackFindingAlgCfg(flags,
                               name: str = "ActsTrackFindingAlg",
                               **kwargs) -> ComponentAccumulator:
    def tolist(c):
        return c if isinstance(c, list) else [c]

    acc = ComponentAccumulator()

    from ActsConfig.ActsGeometryConfig import ActsVolumeIdToDetectorCollectionMappingAlgCfg
    # Remove HGTD Volumes from the propagation unless we need it
    if not flags.Acts.useHGTDClusterInTrackFinding:
        # HGTD has volume id:
        # 2 for negative endcap
        # 25 for positive endcap
        kwargs.setdefault('EndOfTheWorldVolumeIds', [2, 25])
    
    acc.merge( ActsVolumeIdToDetectorCollectionMappingAlgCfg(flags) )
    kwargs.setdefault("ActsVolumeIdToDetectorElementCollectionMapKey", "VolumeIdToDetectorElementCollectionMap")
    def filterCollections(flags, pixel_col, strip_col) :
      ret=[]
      if flags.Detector.GeometryITkPixel:
        ret += [ pixel_col ]
      if flags.Detector.GeometryITkStrip:
        ret += [ strip_col ]
      return ret
    kwargs.setdefault("DetElStatus",filterCollections(flags,'ITkStripDetectorElementStatus','ITkPixelDetectorElementStatus'))

    # Seed labels and collections.
    # These 3 lists must match element for element, reversed if flags.Acts.useStripSeedsFirst is True.
    # Maybe it is best to start with strips where the occupancy is lower.
    kwargs.setdefault("SeedLabels", seedOrder(flags, pixel=["PPP"], strip=["SSS"]))
    kwargs.setdefault("SeedContainerKeys", seedOrder(flags, pixel=["ActsPixelSeeds"], strip=["ActsStripSeeds"]))
    kwargs.setdefault('DetectorElementsKeys', seedOrder(flags, pixel=['ITkPixelDetectorElementCollection'], strip=['ITkStripDetectorElementCollection']))

    kwargs.setdefault("UncalibratedMeasurementContainerKeys", isdet(flags, pixel=["ITkPixelClusters_Cached" if flags.Acts.useCache else "ITkPixelClusters"], strip=["ITkStripClusters_Cached" if flags.Acts.useCache else "ITkStripClusters"], hgtd=["HGTD_Clusters"]))

    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks')

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    kwargs.setdefault("refitSeeds", seedOrder(flags, pixel=[False], strip=[False]))
    kwargs.setdefault("doTwoWay", flags.Acts.doTwoWayCKF)
    kwargs.setdefault("autoReverseSearch", flags.Acts.autoReverseSearchCKF)

    # Borrow many settings from flags.Tracking.ActiveConfig, normally initialised in createITkTrackingPassFlags() at
    # https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkConfig/python/TrackingPassFlags.py#L121

    # bins in |eta|, used for both MeasurementSelectorConfig and TrackSelector::EtaBinnedConfig
    if flags.Detector.GeometryITk:
        kwargs.setdefault("etaBins", flags.Tracking.ActiveConfig.etaBins)
    # new default chi2 cuts optimise efficiency vs speed. Set same value as Athena's Xi2maxNoAdd.
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    if isFastPrimaryPass(flags):
        kwargs.setdefault("chi2CutOff", [100])
        kwargs.setdefault("chi2OutlierCutOff", [100])
    else:
        kwargs.setdefault("chi2CutOff", [25])
        kwargs.setdefault("chi2OutlierCutOff", [25])
    kwargs.setdefault("branchStopperPtMinFactor", 0.9)
    kwargs.setdefault("branchStopperAbsEtaMaxExtra", 0.1)

    kwargs.setdefault("numMeasurementsCutOff", [1])

    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized by abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins", [eta/10. for eta in range(5, 40, 5)]) # eta 0.0 - 4.0 in steps of 0.5

    kwargs.setdefault("absEtaMax", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("ptMin", [p / Units.GeV * UnitConstants.GeV for p in tolist(flags.Tracking.ActiveConfig.minPT)])
    # z0 cut is the same for all eta bins. I use the size of the eta bins limits minus one to find the number of bins.
    kwargs.setdefault("z0Min", [-flags.Tracking.ActiveConfig.maxZImpactSeed / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("z0Max", [ flags.Tracking.ActiveConfig.maxZImpactSeed / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("d0Min", [-d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpact)])
    kwargs.setdefault("d0Max", [ d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpact)])
    kwargs.setdefault("minMeasurements", tolist(flags.Tracking.ActiveConfig.minClusters))
    kwargs.setdefault("maxHoles", tolist(flags.Tracking.ActiveConfig.maxHoles))
    kwargs.setdefault("minPixelHits", tolist(flags.Tracking.ActiveConfig.minPixel))
    kwargs.setdefault("maxPixelHoles", tolist(flags.Tracking.ActiveConfig.maxPixelHoles))
    kwargs.setdefault("maxStripHoles", tolist(flags.Tracking.ActiveConfig.maxSctHoles))
    # The shared hits are not calculated until *after* the track selection, so maxSharedHits is not used.
    # Even if that were not the case, we need the ambiguity solver to decide which track to drop.
    ### kwargs.setdefault("maxSharedHits", tolist(flags.Tracking.ActiveConfig.maxShared))
    kwargs.setdefault("ptMinMeasurements", isdet(flags, pixel=[3], strip=[6]))
    kwargs.setdefault("absEtaMaxMeasurements", isdet(flags, pixel=[3], strip=[999999]))

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault('ATLASConverterTool', acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

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
            acc.popToolsAndMerge(ActsTrackStatePrinterToolCfg(flags)),
        )
 
    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg 
        kwargs.setdefault(
            'FitterTool',
            acc.popToolsAndMerge(ActsFitterCfg(flags, 
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=float('inf')))
        )

    if 'PixelCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType
        from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy
        from ActsConfig.ActsMeasurementCalibrationConfig import ActsAnalogueClusteringToolCfg

        if not (flags.Tracking.doPixelDigitalClustering or flags.Beam.Type is BeamType.Cosmics):
            if flags.Acts.PixelCalibrationStrategy in (PixelCalibrationStrategy.AnalogueClustering,
                                                       PixelCalibrationStrategy.AnalogueClusteringAfterSelection) :
                kwargs.setdefault(
                    'PixelCalibrator',
                    acc.popToolsAndMerge(ActsAnalogueClusteringToolCfg(flags,
                                                                       CalibrateAfterMeasurementSelection = flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClusteringAfterSelection))
                )
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc



def ActsTrackFindingCfg(flags,
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Define Uncalibrated Measurement keys
    dataPrepPrefix = f'{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}'
    if not flags.Tracking.ActiveConfig.isSecondaryPass:
        dataPrepPrefix = ''
    pixelClusters = f'ITk{dataPrepPrefix}PixelClusters'
    stripClusters = f'ITk{dataPrepPrefix}StripClusters'    
    hgtdClusters = f'{dataPrepPrefix}HGTD_Clusters'
    # If cache is activated the keys have "_Cached" as postfix
    if flags.Acts.useCache:
        pixelClusters += '_Cached'
        stripClusters += '_Cached'
    # Consider case detectors are not active

    # Understand what are the seeds we need to consider
    pixelSeedLabels = ['PPP']
    stripSeedLabels = ['SSS']
    # Conversion and LRT do not process pixel seeds
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    if flags.Tracking.ActiveConfig.extension in ['ActsConversion', 'ActsLargeRadius']:
        pixelSeedLabels = None
    # Main pass does not process strip seeds in the fast tracking configuration
    elif isFastPrimaryPass(flags):
        stripSeedLabels = None

    # Now set the seed and estimated parameters keys accordingly
    pixelSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}PixelSeeds']
    stripSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}StripSeeds']
    pixelDetElements = ['ITkPixelDetectorElementCollection']
    stripDetElements = ['ITkStripDetectorElementCollection']
    if pixelSeedLabels is None:
        pixelSeedKeys = None
        pixelDetElements = None
    if stripSeedLabels is None:
        stripSeedKeys = None
        stripDetElements = None

    kwargs.setdefault('ACTSTracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('UncalibratedMeasurementContainerKeys', isdet(flags, pixel=[pixelClusters], strip=[stripClusters], hgtd=[hgtdClusters]))
    kwargs.setdefault('SeedLabels', seedOrder(flags, pixel=pixelSeedLabels, strip=stripSeedLabels))
    kwargs.setdefault('SeedContainerKeys', seedOrder(flags, pixel=pixelSeedKeys, strip=stripSeedKeys))
    kwargs.setdefault('DetectorElementsKeys', seedOrder(flags, pixel=pixelDetElements, strip=stripDetElements))

    acc.merge(ActsMainTrackFindingAlgCfg(flags,
                                         name=f"{flags.Tracking.ActiveConfig.extension}TrackFindingAlg",
                                         **kwargs))

    # Analysis extensions
    if flags.Acts.Tracks.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
        acc.merge(ActsTrackAnalysisAlgCfg(flags,
                                          name=f"{flags.Tracking.ActiveConfig.extension}TrackAnalysisAlg",
                                          TracksLocation=f"{flags.Tracking.ActiveConfig.extension}Tracks"))

    # Persistification
    if flags.Acts.EDM.PersistifyTracks:
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


def ActsMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsScoreBasedAmbiguityResolutionAlg",
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
    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
    acc.addEventAlgo(
        CompFactory.ActsTrk.ScoreBasedAmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsMainAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsAmbiguityResolutionAlg",
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
    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
    acc.addEventAlgo(
        CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsAmbiguityResolutionCfg(flags,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('ResolvedTracksLocation', f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks")
    from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy
            
    if flags.Acts.AmbiguitySolverStrategy is AmbiguitySolverStrategy.ScoreBased:
        acc.merge(ActsMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                                    name=f"{flags.Tracking.ActiveConfig.extension}ScoreBasedAmbiguityResolutionAlg",
                                                    **kwargs))
    else:
        acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags,
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

def ActsTrackToTrackParticleCnvAlgCfg(flags,
                                      name: str = "ActsTrackToTrackParticleCnvAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault('ExtrapolationTool', acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags)) )

    kwargs.setdefault('BeamSpotKey', 'BeamSpotData')
    kwargs.setdefault('FirstAndLastParameterOnly',True)

    det_elements=[]
    element_types=[]
    if flags.Detector.EnableITkPixel:
        det_elements += ['ITkPixelDetectorElementCollection']
        element_types += [1]
    if flags.Detector.EnableITkStrip:
        det_elements += ['ITkStripDetectorElementCollection']
        element_types += [2]

    kwargs.setdefault('SiDetectorElementCollections',det_elements)
    kwargs.setdefault('SiDetEleCollToMeasurementType',element_types)
    kwargs.setdefault("PerigeeExpression", flags.Tracking.perigeeExpression)
    kwargs.setdefault('VertexContainerKey', 'PrimaryVertices')
    acc.addEventAlgo(
        CompFactory.ActsTrk.TrackToTrackParticleCnvAlg(name, **kwargs))

    return acc


