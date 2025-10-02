# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsFullScanRoICreatorToolCfg(flags,
                                  name: str = "ActsFullScanRoICreatorTool",
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.FullScanRoICreatorTool(name, **kwargs))
    return acc

def CaloBasedRoICreatorToolCfg(flags,
                               name : str = "ActsCaloBasedRoICreatorTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('CaloClusterRoIContainer', 'ITkCaloClusterROIPhiRZ15GeVUnordered')
    acc.setPrivateTools(CompFactory.ActsTrk.CaloBasedRoICreatorTool(name, **kwargs))
    return acc

def ActsMainRegionsOfInterestCreatorAlgCfg(flags,
                                           name: str = "ActsMainRegionsOfInterestCreatorAlg",
                                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    if 'RoICreatorTool' not in kwargs:
        kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ActsFullScanRoICreatorToolCfg(flags)))

    kwargs.setdefault('RoIs', 'ActsRegionOfInterest')
    acc.addEventAlgo(CompFactory.ActsTrk.RegionsOfInterestCreatorAlg(name, **kwargs))    
    return acc

def ActsRegionsOfInterestCreatorAlgCfg(flags,
                                       name: str = "ActsRegionsOfInterestCreatorAlg",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Large Radius tracking pass will use the full-scan roi made from primary pass
    if flags.Tracking.ActiveConfig.extension == 'ActsLargeRadius':
        return acc
    
    # Conversion tracking pass requirements
    if flags.Tracking.ActiveConfig.extension == "ActsConversion":
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))

    # Set proper RoI creation tool
    if 'RoICreatorTool' not in kwargs:
        if flags.Tracking.ActiveConfig.extension == "ActsConversion":
            kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(CaloBasedRoICreatorToolCfg(flags)))
        else:
            kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ActsFullScanRoICreatorToolCfg(flags)))

    kwargs.setdefault('RoIs', f"{flags.Tracking.ActiveConfig.extension}RegionOfInterest")
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags, name, **kwargs))
    return acc

def ActsInDetRegionsOfInterestCreatorAlgCfg(flags,
                                       name: str = "ActsInDetRegionsOfInterestCreatorAlg",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Set proper RoI creation tool
    if 'RoICreatorTool' not in kwargs:
        kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(ActsFullScanRoICreatorToolCfg(flags)))

    kwargs.setdefault('RoIs', "RegionOfInterest")
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags, name, **kwargs))
    return acc