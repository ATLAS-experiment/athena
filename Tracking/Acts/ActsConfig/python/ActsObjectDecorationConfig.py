# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMeasurementToTrackParticleDecorationAlgCfg(flags,
                                                   name: str = "ActsMeasurementToTrackParticleDecorationAlg",
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
    
    acc.addEventAlgo(CompFactory.ActsTrk.MeasurementToTrackParticleDecorationAlg(name, **kwargs))
    return acc


def ActsPixelClusterTruthDecoratorAlgCfg(flags,
                                         name: str = "ActsPixelClusterTruthDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","ITkPixelClusters")
    kwargs.setdefault("AssociationMapOut","ITkPixelClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","ITkPixelMeasurements")
    kwargs.setdefault("UseTruthInfo", flags.Tracking.doTruth)
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterTruthDecoratorAlg(name,**kwargs))

    # add SDO and SiHit info
    if flags.Acts.decoratePRD.sdoSiHit:
        acc.merge(ActsPixelClusterSiHitDecoratorAlgCfg(flags))
    
    # Persistification
    if flags.Tracking.writeExtendedSi_PRDInfo:
        toAOD = [
            f'xAOD::TrackMeasurementValidationContainer#{kwargs["MeasurementContainer"]}',
            f'xAOD::TrackMeasurementValidationAuxContainer#{kwargs["MeasurementContainer"]}Aux.'
        ]        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))
        
    return acc


def ActsStripClusterTruthDecoratorAlgCfg(flags,
                                         name: str = "ActsStripClusterTruthDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","ITkStripClusters")
    kwargs.setdefault("AssociationMapOut","ITkStripClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","ITkStripMeasurements")
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterTruthDecoratorAlg(name,**kwargs))

    if flags.Acts.decoratePRD.sdoSiHit:
        acc.merge(ActsStripClusterSiHitDecoratorAlgCfg(flags))
        
    # Persistification
    if flags.Tracking.writeExtendedSi_PRDInfo:
        toAOD = [
            f'xAOD::TrackMeasurementValidationContainer#{kwargs["MeasurementContainer"]}',
            f'xAOD::TrackMeasurementValidationAuxContainer#{kwargs["MeasurementContainer"]}Aux.'
        ]        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))

    return acc

def ActsPixelClusterSiHitDecoratorAlgCfg(flags,
                                         name: str = "ActsPixelClusterSiHitDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Measurements', 'ITkPixelMeasurements')
    kwargs.setdefault('Clusters', 'ITkPixelClusters')
    kwargs.setdefault('SDOs', 'ITkPixelSDO_Map')
    kwargs.setdefault('SiHits', 'ITkPixelHits')
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterSiHitDecoratorAlg(name, **kwargs))
    return acc

def ActsStripClusterSiHitDecoratorAlgCfg(flags,
                                         name: str = "ActsStripClusterSiHitDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Measurements', 'ITkStripMeasurements')
    kwargs.setdefault('Clusters', 'ITkStripClusters')
    kwargs.setdefault('SDOs', 'ITkStripSDO_Map')
    kwargs.setdefault('SiHits', 'ITkStripHits')
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterSiHitDecoratorAlg(name, **kwargs))
    return acc
