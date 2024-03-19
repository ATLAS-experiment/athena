# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from ActsInterop import UnitConstants

# Tools

def isdet(flags, pixel, strip):
    keys = []
    if flags.Detector.EnableITkPixel:
        keys += pixel
    if flags.Detector.EnableITkStrip:
        keys += strip
    return keys

def ActsTrackStatePrinterCfg(flags,
                             name: str = "ActsTrackStatePrinterTool",
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("InputSpacePoints", isdet(flags, ["ITkPixelSpacePoints"], ["ITkStripSpacePoints", "ITkStripOverlapSpacePoints"]))

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinter(name, **kwargs))
    return acc

# ACTS only algorithm

def ActsMainTrackFindingAlgCfg(flags,
                               name: str = "ActsTrackFindingAlg",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Seed labels and collections. These 3 lists must match element for element.
    kwargs.setdefault("SeedLabels", isdet(flags, ["PPP"], ["SSS"]))
    kwargs.setdefault("EstimatedTrackParametersKeys", isdet(flags, ["ITkPixelEstimatedTrackParams"], ["ITkStripEstimatedTrackParams"]))
    kwargs.setdefault("SeedContainerKeys", isdet(flags, ["ITkPixelSeeds"], ["ITkStripSeeds"]))
    # Measurement collections. These 2 lists must match element for element.
    kwargs.setdefault("UncalibratedMeasurementContainerKeys", isdet(flags, ["ITkPixelClusters"], ["ITkStripClusters"]))
    kwargs.setdefault("DetectorElementCollectionKeys", isdet(flags, ["ITkPixelDetectorElementCollection"], ["ITkStripDetectorElementCollection"]))

    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks')

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    # bins in |eta|, used for both MeasurementSelectorCuts and TrackSelector::EtaBinnedConfig
    if flags.Detector.GeometryITk:
        kwargs.setdefault("etaBins", flags.Tracking.ActiveConfig.etaBins)
    kwargs.setdefault("chi2CutOff", [flags.Acts.trackFindingChi2CutOff])
    kwargs.setdefault("numMeasurementsCutOff", [1])

    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized by abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins", [eta/10. for eta in range(5, 40, 5)]) # eta 0.0 - 4.0 in steps of 0.5

    if flags.Acts.doTrackFindingTrackSelector:
        def tolist(c):
            return c if isinstance(c, list) else [c]
        # Use settings from flags.Tracking.ActiveConfig, initialised in createITkTrackingPassFlags() at
        # https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkConfig/python/TrackingPassFlags.py#L376
        kwargs.setdefault("absEtaMax", flags.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("ptMin",
                          [p / Units.GeV * UnitConstants.GeV for p in tolist(flags.Tracking.ActiveConfig.minPT)])
        kwargs.setdefault("minMeasurements",
                          tolist(flags.Tracking.ActiveConfig.minClusters))
        if flags.Acts.doTrackFindingTrackSelector == 2:
            # use the same cut for all eta - for comparison with previous behaviour
            kwargs["ptMin"] = [min(kwargs["ptMin"])]
            kwargs["minMeasurements"] = [min(kwargs["minMeasurements"])]
        elif flags.Acts.doTrackFindingTrackSelector != 3:
            # include hole/shared hit cuts - disable for comparison with previous behaviour
            kwargs.setdefault("maxHoles", tolist(flags.Tracking.ActiveConfig.maxHoles))
            kwargs.setdefault("maxSharedHits", tolist(flags.Tracking.ActiveConfig.maxShared))
            if flags.Acts.doTrackFindingTrackSelector == 4:
                # don't use branch stopper - for comparison with previous behaviour
                kwargs.setdefault("doBranchHoleCut", False)

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )
        
    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )
        
    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault(
            "ATLASConverterTool",
            acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
        )

    if flags.Acts.doPrintTrackStates and 'TrackStatePrinter' not in kwargs:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsTrackStatePrinterCfg(flags)),
        )
 
    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg 
        kwargs.setdefault(
            'FitterTool',
            acc.popToolsAndMerge(ActsFitterCfg(flags, 
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=30))
        )

    if 'PixelCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType
        from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy
        from ActsConfig.ActsMeasurementCalibrationConfig import ActsAnalogueClusteringToolCfg

        if flags.Beam.Type is not BeamType.Cosmics:
            if flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClustering:
                kwargs.setdefault(
                    'PixelCalibrator',
                    acc.popToolsAndMerge(ActsAnalogueClusteringToolCfg(flags))
                )
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc


def ActsTrackFindingCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Acts Main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        acc.merge(ActsMainTrackFindingAlgCfg(flags,
                                             SeedLabels = isdet(flags, ["PPP"], ["SSS"]) if not flags.Tracking.doITkFastTracking else ["PPP"], 
                                             EstimatedTrackParametersKeys = isdet(flags, ["ITkPixelEstimatedTrackParams"], ["ITkStripEstimatedTrackParams"]) if not flags.Tracking.doITkFastTracking else ["ITkPixelEstimatedTrackParams"],
                                             SeedContainerKeys = isdet(flags, ["ITkPixelSeeds"], ["ITkStripSeeds"]) if not flags.Tracking.doITkFastTracking else ["ITkPixelSeeds"],
                                             UncalibratedMeasurementContainerKeys = isdet(flags, ["ITkPixelClusters_InView"], ["ITkStripClusters_InView"]) if flags.Acts.useCache else isdet(flags, ["ITkPixelClusters"], ["ITkStripClusters"])))
    # Acts Conversion pass
    elif flags.Tracking.ActiveConfig.extension == "ActsConversion":
        acc.merge(ActsMainTrackFindingAlgCfg(flags,
                                             name="ActsConversionTrackFindingAlg",
                                             ACTSTracksLocation="ActsConversionTracks",
                                             SeedLabels=["SSS"],
                                             EstimatedTrackParametersKeys=["ITkConversionStripEstimatedTrackParams"],
                                             SeedContainerKeys=["ITkConversionStripSeeds"],
                                             UncalibratedMeasurementContainerKeys=isdet(flags, ["ITkPixelClusters_InView"], ["ITkConversionStripClusters_InView"]) if flags.Acts.useCache else isdet(flags, ["ITkPixelClusters"], ["ITkConversionStripClusters"])
                                             ))
    # Any other pass -> mainly validation
    else:
        acc.merge(ActsMainTrackFindingAlgCfg(flags))
        
    return acc

def ActsMainAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsAmbiguityResolutionAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ResolvedActsTracks')
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


def ActsAmbiguityResolutionCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Acts Main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags))
    # Acts Conversion pass
    elif flags.Tracking.ActiveConfig.extension == "ActsConversion":
        acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags,
                                                    name="ActsConversionAmbiguityResolution",
                                                    TracksLocation="ActsConversionTracks",
                                                    ResolvedTracksLocation="ResolvedActsConversionTracks"))
    # Any other pass -> mainly validation
    else:
        acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags))
        
    return acc

def ActsTrackToTrackParticleCnvAlgCfg(flags,
                                      name: str = "ActsTrackToTrackParticleCnvAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault('ExtrapolationTool',    acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags)) )

    if flags.Tracking.ActiveConfig.extension == "ActsConversion":
        kwargs.setdefault('ACTSTracksLocation', 'ResolvedActsConversionTracks')
        kwargs.setdefault('TrackParticlesOutKey','ResolvedActsConversionTrackParticles')
    else:
        kwargs.setdefault('ACTSTracksLocation', 'ResolvedActsTracks')
        kwargs.setdefault('TrackParticlesOutKey','ResolvedActsTrackParticles')
        
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
    acc.addEventAlgo(
        CompFactory.ActsTrk.TrackToTrackParticleCnvAlg(name, **kwargs))
    return acc
