# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FullScanRoICreatorToolCfg(flags,
                              name: str = "FullScanRoICreatorTool",
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')
    acc.setPrivateTools(CompFactory.FullScanRoICreatorTool(name, **kwargs))
    return acc

def ConversionRoICreatorToolCfg(flags,
                                name : str = "ConversionRoICreatorTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('CaloClusterRoIContainer', 'ITkCaloClusterROIPhiRZ15GeVUnordered')
    acc.setPrivateTools(CompFactory.CaloBasedRoICreatorTool(name, **kwargs))
    return acc

def GlobalEventViewCreatorAlgCfg(flags,
                                 name: str,
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if 'RoICreatorTool' not in kwargs:
        kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(FullScanRoICreatorToolCfg(flags)))

    kwargs.setdefault('Views', 'OfflineFullScanEventView')
    kwargs.setdefault('InViewRoIs', 'OfflineFullScanInViewRegion')
    acc.addEventAlgo(CompFactory.EventViewCreatorAlg(name, **kwargs))
    return acc

def CaloBasedEventViewCreatorAlgCfg(flags,
                                    name: str,
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if 'RoICreatorTool' not in kwargs:
         kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ConversionRoICreatorToolCfg(flags)))

    kwargs.setdefault('Views', 'OfflineCaloBasedEventView')
    kwargs.setdefault('InViewRoIs', 'OfflineCaloBasedInViewRegion')
    acc.addEventAlgo(CompFactory.EventViewCreatorAlg(name, **kwargs))
    return acc

def EventViewCreatorAlgCfg(flags,
                           name: str = "EventViewCreatorAlg",
                           **kwargs) -> ComponentAccumulator:
    # Acts main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        return GlobalEventViewCreatorAlgCfg(flags, name=name, **kwargs)
    # Acts conversion pass
    if flags.Tracking.ActiveConfig.extension == "ActsConversion":
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
        acc = ITkCaloClusterROIPhiRZContainerMakerCfg(flags)
        acc.merge(CaloBasedEventViewCreatorAlgCfg(flags, name, **kwargs))
        return acc
    # Any other Acts pass, that means validation passes
    return GlobalEventViewCreatorAlgCfg(flags, name=name, **kwargs)

        
