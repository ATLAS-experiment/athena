# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsHGTDTrackExtensionAlgConfig(
    flags,
    name: str = "ActsHGTDTrackExtensionAlg",
    enableTrackStatePrinter: bool = False,
    **kwargs
) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if flags.Acts.doMonitoring and "MonTools" not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsHGTDTrackExtensionMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsHGTDTrackExtensionMonitoringCfg(flags)))

    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")
    kwargs.setdefault("HGTDClusterContainerName", "HGTD_Clusters")
    kwargs.setdefault("UncalibratedMeasurementContainerKey_HGTD", "HGTD_Clusters")

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=1e20)),
        )

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    
    if enableTrackStatePrinter:
        from ActsConfig.ActsTrackFindingConfig import ActsTrackStatePrinterCfg
        kwargs.setdefault("TrackStatePrinter", acc.popToolsAndMerge(ActsTrackStatePrinterCfg(flags)))

    HGTDTrackExtensionAlg = CompFactory.ActsTrk.HGTDTrackExtensionAlg(name, **kwargs)
    
    acc.addEventAlgo(HGTDTrackExtensionAlg)

    return acc
