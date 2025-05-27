    #  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackExtensionAlgCfg(flags,enableTrackStatePrinter=False, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelClusterContainer", "ITkPixelClusters")
    kwargs.setdefault("ACTSTracksLocation", "ExtendedFPGATracks")
    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=100)),
        )

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs["TrackingGeometryTool"] = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))

    if 'ActsFitter' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
        kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(ActsFitterCfg(flags,
                                                                           ReverseFilteringPt=0,
                                                                           OutlierChi2Cut=30)))
    if enableTrackStatePrinter:
        from ActsConfig.ActsTrackFindingConfig import ActsTrackStatePrinterToolCfg
        printerTool = acc.popToolsAndMerge(ActsTrackStatePrinterToolCfg(flags))
        kwargs["TrackStatePrinter"] = printerTool 

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    
    acc.addEventAlgo(CompFactory.ActsTrk.TrackExtensionAlg(**kwargs))
    return acc
