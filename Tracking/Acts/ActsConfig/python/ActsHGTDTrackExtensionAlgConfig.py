# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsHGTDTrackExtensionAlgCfg(flags,
                                 name: str = "ActsHGTDTrackExtensionAlg",
                                 *,
                                 enableTrackStatePrinter: bool = False,
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")
    kwargs.setdefault("HGTDClusterContainerName", "HGTD_Clusters")
    kwargs.setdefault("UncalibratedMeasurementContainerKey_HGTD", "HGTD_Clusters")

    if flags.Acts.doMonitoring and "MonTools" not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsHGTDTrackExtensionMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsHGTDTrackExtensionMonitoringCfg(flags)))

    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags,
                                                                                             MaxSteps = 10000)))

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    
    if enableTrackStatePrinter and 'TrackStatePrinter' not in kwargs:
        from ActsConfig.ActsTrackFindingConfig import ActsTrackStatePrinterToolCfg
        kwargs.setdefault("TrackStatePrinter", acc.popToolsAndMerge(ActsTrackStatePrinterToolCfg(flags)))
        
    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTrackExtensionAlg(name, **kwargs))
    return acc

def HGTDTruthTrackDecorationAlgCfg(flags,
                                   name: str = "HGTDTruthTrackDecorationAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")
    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTruthTrackDecorationAlg(name, **kwargs))
    return acc

