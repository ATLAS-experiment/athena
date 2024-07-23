    #  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackExtensionAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PixelClusterContainer", "ITkPixelClusters")
    kwargs.setdefault("ACTSTracksLocation", "ExtendedFPGATracks")
    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=100)),
        )
    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs["ATLASConverterTool"] = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs["TrackingGeometryTool"] = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackExtensionAlg(**kwargs))
    return acc