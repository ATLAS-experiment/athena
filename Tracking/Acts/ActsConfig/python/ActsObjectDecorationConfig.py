# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMeasurementToTrackParticleDecorationCfg(flags,
                                                name: str = "ActsMeasurementToTrackParticleDecoration",
                                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackParticleKey", "InDetTrackParticles")

    # TODO:: The tracking geometry tool is not strictly necessary
    # but can provide extra information on surfaces if needed in the future
    
    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )
    

    acc.addEventAlgo(CompFactory.ActsTrk.MeasurementToTrackParticleDecoration(name, **kwargs))
    return acc

