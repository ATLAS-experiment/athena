# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsTrackStateOnSurfaceDecoratorAlgCfg(flags,
                                           name: str = "ActsTrackStateOnSurfaceDecoratorAlg",
                                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TrackParticles', 'InDetTrackParticles')
    kwargs.setdefault('PixelMSOSs', 'ITkPixelMSOSs')
    kwargs.setdefault('StripMSOSs', 'ITkStripMSOSs')
    kwargs.setdefault('ExtraInputs',[
        ( 'xAOD::PixelClusterContainer' , 'StoreGateSvc+ITkPixelClusters.validationMeasurementLink' ),
        ( 'xAOD::StripClusterContainer' , 'StoreGateSvc+ITkStripClusters.validationMeasurementLink' ),
        ( 'xAOD::TrackMeasurementValidationContainer' , 'StoreGateSvc+ITkPixelMeasurements' ),
        ( 'xAOD::TrackMeasurementValidationContainer' , 'StoreGateSvc+ITkStripMeasurements' )
    ])
    
    acc.addEventAlgo(CompFactory.ActsTrk.ActsTrackStateOnSurfaceDecoratorAlg(name, **kwargs))

    toAOD = []
    toAOD += [f'xAOD::TrackStateValidationContainer#{kwargs["PixelMSOSs"]}',
              f'xAOD::TrackStateValidationAuxContainer#{kwargs["PixelMSOSs"]}Aux.',
              f'xAOD::TrackStateValidationContainer#{kwargs["StripMSOSs"]}',
              f'xAOD::TrackStateValidationAuxContainer#{kwargs["StripMSOSs"]}Aux.']

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc


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
            acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )
    
    acc.addEventAlgo(CompFactory.ActsTrk.MeasurementToTrackParticleDecorationAlg(name, **kwargs))
    return acc


def ActsPixelClusterTruthDecoratorAlgCfg(flags,
                                         name: str = "ActsPixelClusterTruthDecoratorAlg",
                                         *,
                                         TrackParticles: list[str] = None,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","ITkPixelClusters")
    kwargs.setdefault("AssociationMapOut","ITkPixelClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","ITkPixelMeasurements")
    kwargs.setdefault("UseTruthInfo", flags.Tracking.doTruth)
    
    if flags.Tracking.PRDInfo.KeepOnlyOnTrackMeasurements:
        if TrackParticles is None:
            raise ValueError("Requesting persistification of on-track clusters, but no track particle collection has been provided!")
        
        kwargs.setdefault("KeepOnlyOnTrackMeasurements", True)
        kwargs.setdefault("TrackParticles", TrackParticles)

        deps = []
        for collection in TrackParticles:
            deps += [( 'xAOD::TrackParticleContainer' , f'StoreGateSvc+{collection}.actsTrack' )]
        kwargs.setdefault('ExtraInputs', deps)

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge( ITkPixelLorentzAngleToolCfg(flags) ))
        
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
                                         *,
                                         TrackParticles: list[str] = None,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","ITkStripClusters")
    kwargs.setdefault("AssociationMapOut","ITkStripClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","ITkStripMeasurements")

    if flags.Tracking.PRDInfo.KeepOnlyOnTrackMeasurements:
        if TrackParticles is None:
            raise ValueError("Requesting persistification of on-track clusters, but no track particle collection has been provided!")

        kwargs.setdefault("KeepOnlyOnTrackMeasurements", True)
        kwargs.setdefault("TrackParticles", TrackParticles)

        deps = []
        for collection in TrackParticles:
            deps += [( 'xAOD::TrackParticleContainer' , f'StoreGateSvc+{collection}.actsTrack' )]
        kwargs.setdefault('ExtraInputs', deps)
            
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
    kwargs.setdefault('SDOs', 'ITkPixelSDO_Map')
    kwargs.setdefault('SiHits', 'ITkPixelHits')
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterSiHitDecoratorAlg(name, **kwargs))
    return acc

def ActsStripClusterSiHitDecoratorAlgCfg(flags,
                                         name: str = "ActsStripClusterSiHitDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Measurements', 'ITkStripMeasurements')
    kwargs.setdefault('SDOs', 'ITkStripSDO_Map')
    kwargs.setdefault('SiHits', 'ITkStripHits')
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterSiHitDecoratorAlg(name, **kwargs))
    return acc
