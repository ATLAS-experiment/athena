#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ActsGaussianSumFitterToolCfg(flags,
                                 name: str = "ActsGaussianSumFitterTool",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("UseDirectNavigation", flags.Acts.GsfDirectNavigation) # direct navigation used for refitting measurements
    kwargs.setdefault("ComponentMergeMethod", flags.Acts.GsfComponentMergeMethod) # Mean or MaxWeight
    kwargs.setdefault("MaxComponents", flags.Acts.GsfMaxComponents)
    kwargs.setdefault("OutlierChi2Cut", flags.Acts.GsfOutlierChi2Cut)

    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    from ActsConfig.ActsConfigFlags import TrackFitterType
    the_tool = acc.popToolsAndMerge(ActsFitterCfg(flags,
                                                  fitterKind = TrackFitterType.GaussianSumFitter,
                                                  name=name,
                                                  **kwargs))

    acc.setPrivateTools(the_tool)
    return acc

def ActsTrkGaussianSumFitterToolCfg(flags,
                                    name: str = "ActsTrkGaussianSumFitterTool",
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("ActsFitterTool", 
                      acc.popToolsAndMerge(ActsGaussianSumFitterToolCfg(flags)))
    
    from ActsConfig.ActsTrackFittingConfig import ActsToTrkFitterCfg
    the_tool = acc.popToolsAndMerge(ActsToTrkFitterCfg(flags, name, **kwargs))

    acc.setPrivateTools(the_tool)
    return acc


