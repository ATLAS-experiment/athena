#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsGaussianSumFitterToolCfg(flags,
                                 name: str = "ActsGaussianSumFitterTool",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("RefitOnly", True) # Track summary will be added in the algorithm

    kwargs.setdefault("UseDirectNavigation", flags.Acts.GsfDirectNavigation) # direct navigation used for refitting measurements
    kwargs.setdefault("ComponentMergeMethod", flags.Acts.GsfComponentMergeMethod) # Mean or MaxWeight
    kwargs.setdefault("MaxComponents", flags.Acts.GsfMaxComponents)
    kwargs.setdefault("OutlierChi2Cut", flags.Acts.GsfOutlierChi2Cut)

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs["TrackingGeometryTool"] = acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs["ExtrapolationTool"] = acc.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags, MaxSteps=10000)
        ) # PrivateToolHandle

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs["ATLASConverterTool"] = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    if 'BoundaryCheckTool' not in kwargs:    
        if flags.Detector.GeometryITk:
            from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg            
            kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags)))
        else:
            from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
            kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.GaussianSumFitterTool(name, **kwargs))
    return acc


