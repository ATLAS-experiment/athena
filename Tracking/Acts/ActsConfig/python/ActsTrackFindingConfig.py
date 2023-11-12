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

def ActsTrackStatePrinterCfg(
    flags, name: str = "TrackStatePrinter", **kwargs
) -> ComponentAccumulator:
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

def ActsTrackFindingCfg(flags,
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

    if flags.Acts.doAmbiguityResolution:
        kwargs.setdefault(
            "ACTSTracksLocation",
            "ActsTracks"
        )

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    # bins in |eta|, used for both MeasurementSelectorCuts and TrackSelector::EtaBinnedConfig
    if flags.Detector.GeometryITk:
        kwargs.setdefault("etaBins", flags.Tracking.ActiveConfig.etaBins)
    kwargs.setdefault("chi2CutOff", [flags.Acts.trackFindingChi2CutOff])
    kwargs.setdefault("numMeasurementsCutOff", [3])
    kwargs.setdefault("maxHoles", flags.Acts.trackFindingMaxHoles)

    if flags.Acts.doTrackFindingTrackSelector:
        def tolist(c):
            return c if isinstance(c, list) else [c]
        kwargs.setdefault("absEtaMax", flags.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("ptMin",
                          [p / Units.GeV * UnitConstants.GeV for p in tolist(flags.Tracking.ActiveConfig.minPT)])
        kwargs.setdefault("minMeasurements",
                          tolist(flags.Tracking.ActiveConfig.minClusters))
        if flags.Acts.doTrackFindingTrackSelector != 3:
            kwargs.setdefault("maxHolesSelect", tolist(flags.Tracking.ActiveConfig.maxHoles))
            kwargs.setdefault("maxSharedHits", tolist(flags.Tracking.ActiveConfig.maxShared))
        if flags.Acts.doTrackFindingTrackSelector == 2:
            # use the same cut for all eta for comparison with previous behaviour
            kwargs["ptMin"] = [min(kwargs["ptMin"])]
            kwargs["minMeasurements"] = [min(kwargs["minMeasurements"])]

    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg, ActsTrackingGeometryToolCfg
    kwargs.setdefault(
        "TrackingGeometryTool",
        acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
    )  # PrivateToolHandle
    kwargs.setdefault(
        "ExtrapolationTool",
        acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    )  # PrivateToolHandle

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    if flags.Acts.doPrintTrackStates:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsTrackStatePrinterCfg(flags)),
        )
    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized by abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins", [eta/10. for eta in range(5, 40, 5)]) # eta 0.0 - 4.0 in steps of 0.5
    kwargs.setdefault("DumpEtaBinsForAll", False)
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD
    collections = ["xAOD::TrackSummaryContainer#SiSPSeededActsTrackTrackSummary", "xAOD::TrackSummaryAuxContainer#SiSPSeededActsTrackTrackSummaryAux."] 
    collections += ["xAOD::TrackStateContainer#SiSPSeededActsTrackStates", "xAOD::TrackStateAuxContainer#SiSPSeededActsTrackStatesAux."]
    collections += ["xAOD::TrackParametersContainer#SiSPSeededActsTrackParameters", "xAOD::TrackParametersAuxContainer#SiSPSeededActsTrackParametersAux."]
    collections += ["xAOD::TrackJacobianContainer#SiSPSeededActsTrackJacobians", "xAOD::TrackJacobianAuxContainer#SiSPSeededActsTrackJacobiansAux."]
    collections += ["xAOD::TrackMeasurementContainer#SiSPSeededActsTrackMeasurements", "xAOD::TrackMeasurementAuxContainer#SiSPSeededActsTrackMeasurementsAux."]
    collections += ["xAOD::TrackSurfaceContainer#SiSPSeededActsTrackStateSurfaces", "xAOD::TrackSurfaceAuxContainer#SiSPSeededActsTrackStateSurfacesAux."]
    collections += ["xAOD::TrackSurfaceContainer#SiSPSeededActsTrackSurfaces", "xAOD::TrackSurfaceAuxContainer#SiSPSeededActsTrackSurfacesAux."]

    acc.merge(addToESD(flags, collections))

    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg 
        kwargs.setdefault(
            'FitterTool',
            acc.popToolsAndMerge(ActsFitterCfg(flags, 
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=30))
        )

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc


def ActsAmbiguityResolutionCfg(flags, name: str = "ActsAmbiguityResolution", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ResolvedActsTracks')
    kwargs.setdefault('MaximumSharedHits', 3)
    kwargs.setdefault('MaximumIterations', 10000)
    kwargs.setdefault('NMeasurementsMin', 7)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc

def ActsTrackToTrackParticleCnvAlgCfg(flags, name: str = "ActsTrackToTrackParticleCnvAlg", **kwargs):
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault('ExtrapolationTool',    acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags)) )

    kwargs.setdefault('ACTSTracksLocation', 'ResolvedActsTracks')
    kwargs.setdefault('BeamSpotKey', 'BeamSpotData')
    kwargs.setdefault('TrackParticlesOutKey','ResolvedActsTrackParticles')
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
