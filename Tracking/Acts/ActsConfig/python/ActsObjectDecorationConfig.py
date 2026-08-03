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


def ActsPLRClusterMeasurementDecoratorAlgCfg(flags,
                                             name: str = "ActsPLRClusterMeasurementDecoratorAlg",
                                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer", "PLR_Clusters")
    kwargs.setdefault("MeasurementContainer", "PLRMeasurements")
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("IDHelperName", "PLR_ID")
    kwargs.setdefault("MeasurementSizeZ", "sizeR")
    kwargs.setdefault("UseTruthInfo", False)
    kwargs.setdefault("KeepOnlyOnTrackMeasurements", False)

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.PLR_LorentzAngleConfig import PLR_LorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool",
                          acc.popToolsAndMerge(PLR_LorentzAngleToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterTruthDecoratorAlg(name, **kwargs))

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

# ActsInDet... -- Run 3 Silicon Tracking specific decorators
def ActsInDetTrackStateOnSurfaceDecoratorAlgCfg(flags,
                                           name: str = "ActsInDetTrackStateOnSurfaceDecoratorAlg",
                                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TrackParticles', 'InDetTrackParticles')
    kwargs.setdefault('PixelMSOSs', 'PixelMSOSs')
    kwargs.setdefault('StripMSOSs', 'StripMSOSs')
    kwargs.setdefault('ExtraInputs',[
        ( 'xAOD::PixelClusterContainer' , 'StoreGateSvc+PixelClusters.validationMeasurementLink' ),
        ( 'xAOD::StripClusterContainer' , 'StoreGateSvc+SCT_Clusters.validationMeasurementLink' ),
        ( 'xAOD::TrackMeasurementValidationContainer' , 'StoreGateSvc+PixelMeasurements' ),
        ( 'xAOD::TrackMeasurementValidationContainer' , 'StoreGateSvc+SCT_Measurements' )
    ])
    kwargs.setdefault('isITk', False)
    
    acc.addEventAlgo(CompFactory.ActsTrk.ActsTrackStateOnSurfaceDecoratorAlg(name, **kwargs))

    toAOD = []
    toAOD += [f'xAOD::TrackStateValidationContainer#{kwargs["PixelMSOSs"]}',
              f'xAOD::TrackStateValidationAuxContainer#{kwargs["PixelMSOSs"]}Aux.',
              f'xAOD::TrackStateValidationContainer#{kwargs["StripMSOSs"]}',
              f'xAOD::TrackStateValidationAuxContainer#{kwargs["StripMSOSs"]}Aux.']

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc

def ActsInDetPixelClusterTruthDecoratorAlgCfg(flags,
                                         name: str = "ActsInDetPixelClusterTruthDecoratorAlg",
                                         *,
                                         TrackParticles: list[str] = None,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","PixelClusters")
    kwargs.setdefault("AssociationMapOut","PixelClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","PixelMeasurements")
    kwargs.setdefault("PixelDetEleCollKey", "PixelDetectorElementCollection")
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
        from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge( PixelLorentzAngleToolCfg(flags) ))
        
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterTruthDecoratorAlg(name,**kwargs))

    # add SDO and SiHit info
    if flags.Acts.decoratePRD.sdoSiHit:
        acc.merge(ActsInDetPixelClusterSiHitDecoratorAlgCfg(flags))
    
    # Persistification
    if flags.Tracking.writeExtendedSi_PRDInfo:
        toAOD = [
            f'xAOD::TrackMeasurementValidationContainer#{kwargs["MeasurementContainer"]}',
            f'xAOD::TrackMeasurementValidationAuxContainer#{kwargs["MeasurementContainer"]}Aux.'
        ]        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))
        
    return acc


def ActsInDetStripClusterTruthDecoratorAlgCfg(flags,
                                         name: str = "ActsInDetStripClusterTruthDecoratorAlg",
                                         *,
                                         TrackParticles: list[str] = None,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterContainer","SCT_Clusters")
    kwargs.setdefault("AssociationMapOut","SCT_ClustersToTruthParticles")
    kwargs.setdefault("MeasurementContainer","SCT_Measurements")
    kwargs.setdefault("StripDetectorElements", "SCT_DetectorElementCollection")

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
        acc.merge(ActsInDetStripClusterSiHitDecoratorAlgCfg(flags))
        
    # Persistification
    if flags.Tracking.writeExtendedSi_PRDInfo:
        toAOD = [
            f'xAOD::TrackMeasurementValidationContainer#{kwargs["MeasurementContainer"]}',
            f'xAOD::TrackMeasurementValidationAuxContainer#{kwargs["MeasurementContainer"]}Aux.'
        ]        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))

    return acc

def ActsInDetPixelClusterSiHitDecoratorAlgCfg(flags,
                                         name: str = "ActsInDetPixelClusterSiHitDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Measurements', 'PixelMeasurements')
    kwargs.setdefault('SDOs', 'PixelSDO_Map')
    kwargs.setdefault('SiHits', 'PixelHits')
    kwargs.setdefault('PixelDetEleCollKey', 'PixelDetectorElementCollection')
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterSiHitDecoratorAlg(name, **kwargs))
    return acc

def ActsInDetStripClusterSiHitDecoratorAlgCfg(flags,
                                         name: str = "ActsInDetStripClusterSiHitDecoratorAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Measurements', 'SCT_Measurements')
    kwargs.setdefault('SDOs', 'SCT_SDO_Map')
    kwargs.setdefault('SiHits', 'SCT_Hits')
    kwargs.setdefault('StripDetEleCollKey', 'SCT_DetectorElementCollection')
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterSiHitDecoratorAlg(name, **kwargs))
    return acc

def ActsGNNScoreDecoratorAlgCfg(flags,
                                name: str = "ActsGNNScoreDecoratorAlg",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ActsTrk.GNNScoreDecoratorAlg(name, **kwargs))
    return acc
