# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsFullScanRoICreatorToolCfg(flags,
                                  name: str = "ActsFullScanRoICreatorTool",
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')
    acc.setPrivateTools(CompFactory.FullScanRoICreatorTool(name, **kwargs))
    return acc

def ActsConversionRoICreatorToolCfg(flags,
                                    name : str = "ActsConversionRoICreatorTool",
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('RoIs', 'OfflineCaloBasedRegion')
    kwargs.setdefault('CaloClusterRoIContainer', 'ITkCaloClusterROIPhiRZ15GeVUnordered')
    acc.setPrivateTools(CompFactory.CaloBasedRoICreatorTool(name, **kwargs))
    return acc

def ActsGlobalEventViewCreatorAlgCfg(flags,
                                     name: str = "ActsGlobalEventViewCreatorAlg",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if 'RoICreatorTool' not in kwargs:
        kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ActsFullScanRoICreatorToolCfg(flags)))

    kwargs.setdefault('Views', 'OfflineFullScanEventView')
    kwargs.setdefault('InViewRoIs', 'OfflineFullScanInViewRegion')
    acc.addEventAlgo(CompFactory.EventViewCreatorAlg(name, **kwargs))
    return acc

def ActsCaloBasedEventViewCreatorAlgCfg(flags,
                                        name: str = "ActsCaloBasedEventViewCreatorAlg",
                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if 'RoICreatorTool' not in kwargs:
         kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ActsConversionRoICreatorToolCfg(flags)))

    kwargs.setdefault('Views', 'OfflineCaloBasedEventView')
    kwargs.setdefault('InViewRoIs', 'OfflineCaloBasedInViewRegion')
    acc.addEventAlgo(CompFactory.EventViewCreatorAlg(name, **kwargs))
    return acc

def ActsEventViewCreatorAlgCfg(flags,
                               name: str = "ActsEventViewCreatorAlg",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    # Acts main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        acc.merge(ActsGlobalEventViewCreatorAlgCfg(flags, name, **kwargs))
    # Acts conversion pass
    elif flags.Tracking.ActiveConfig.extension == "ActsConversion":
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))
        acc.merge(ActsCaloBasedEventViewCreatorAlgCfg(flags, name, **kwargs))
    # Any other Acts pass, that means validation passes
    else:
        acc.merge(ActsGlobalEventViewCreatorAlgCfg(flags, name, **kwargs))

    return acc
