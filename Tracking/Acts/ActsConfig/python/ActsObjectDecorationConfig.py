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


def ActsPixelClusterTruthDecorator(flags,
                                   name: str = "ActsPixelClusterTruthDecorator",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("SiClusterContainer","ITkPixelClusters")
    kwargs.setdefault("AssociationMapOut","ITkPixelClustersToTruthParticles")

    #Using the same name of the xAOD::SiCluster causes bunch of warnings
    kwargs.setdefault("OutputClusterContainer","ITkPixelClusters")


    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterTruthDecorator(name,**kwargs))
    return acc


def ActsStripClusterTruthDecorator(flags,
                                   name: str = "ActsStripClusterTruthDecorator",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("SiClusterContainer","ITkStripClusters")
    kwargs.setdefault("AssociationMapOut","ITkStripClustersToTruthParticles")
    kwargs.setdefault("OutputClusterContainer","ITkStripClusters")


    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterTruthDecorator(name,**kwargs))
    return acc
