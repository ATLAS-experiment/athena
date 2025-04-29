#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsInterop import UnitConstants
from ActsConfig.ActsUtilities import extractChildKwargs

def MapToInDetSimDataWrapSvcCfg(flags,
                                *,
                                collectionName: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    AddressRemappingSvc = CompFactory.AddressRemappingSvc(
        TypeKeyOverwriteMaps = [f"InDetSimDataCollection#{collectionName}->InDetSimDataCollectionWrap#{collectionName}"]
        )
    acc.addService(AddressRemappingSvc)
    return acc


def ActsPixelClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsPixelClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkPixelSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'ITkPixelSDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 300) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'ITkPixelClusters')
    kwargs.setdefault('AssociationMapOut', 'ITkPixelClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.PixelClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsStripClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsStripClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'ITkStripSDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 600) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'ITkStripClusters')
    kwargs.setdefault('AssociationMapOut', 'ITkStripClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.StripClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsHgtdClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsHgtdClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='HGTD_SDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'HGTD_SDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 600) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'HGTD_Clusters')
    kwargs.setdefault('AssociationMapOut', 'HgtdClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.HgtdClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsTrackToTruthAssociationAlgCfg(flags,
                                      name: str = 'ActsTracksToTruthAssociationAlg',
                                      **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )

    kwargs.setdefault('ACTSTracksLocation','ActsTracks')
    kwargs.setdefault('PixelClustersToTruthAssociationMap','ITkPixelClustersToTruthParticles')
    kwargs.setdefault('StripClustersToTruthAssociationMap','ITkStripClustersToTruthParticles')
    kwargs.setdefault('HgtdClustersToTruthAssociationMap','HgtdClustersToTruthParticles')
    kwargs.setdefault('AssociationMapOut','ActsTracksToTruthParticles')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV)

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
        
    acc.addEventAlgo( CompFactory.ActsTrk.TrackToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsTruthParticleHitCountAlgCfg(flags,
                                    name: str = 'ActsTruthParticleHitCountAlg',
                                    **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )

    kwargs.setdefault('PixelClustersToTruthAssociationMap','ITkPixelClustersToTruthParticles')
    kwargs.setdefault('StripClustersToTruthAssociationMap','ITkStripClustersToTruthParticles')
    kwargs.setdefault('HgtdClustersToTruthAssociationMap','HgtdClustersToTruthParticles')
    kwargs.setdefault('TruthParticleHitCountsOut','TruthParticleHitCounts')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV) # @TODO introduce flag and synchronise with TrackToTruthAssociationAlg
    kwargs.setdefault('NHitsMin',4)

    if 'TrackingGeometryTool' not in kwargs:    
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
    
    acc.addEventAlgo( CompFactory.ActsTrk.TruthParticleHitCountAlg(name=name, **kwargs) )
    return acc


def ActsTruthAssociationAlgCfg(flags,
                              **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="PixelClusterToTruthAssociationAlg.", **kwargs) ))
        
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="StripClusterToTruthAssociationAlg.", **kwargs) ))
    
    if flags.Detector.EnableHGTD and (flags.Acts.useHGTDClusterInTrackFinding or flags.HGTD.doActs):
        acc.merge(ActsHgtdClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="HgtdClusterToTruthAssociationAlg.", **kwargs) ))
    return acc

def setDefaultTruthMatchingArgs(kwargs) :
    kwargs.setdefault('MatchWeights',[0.,               # other
                                      10., 5.,           # ID (pixel, strips)
                                      0.,  0., 0. , 0.,  # MS
                                      0. ])             # HGTD
    # weights used for hit purity and hit efficiencies
    kwargs.setdefault('CountWeights',[0.,               # other
                                      1.,1.,            # ID (pixel, strips)
                                      0., 0., 0. , 0.,  # MS
                                      0. ])             # HGTD
    kwargs.setdefault('StatisticPtBins',[1e3,2.5e3,5e3,10e3,100e3])
    kwargs.setdefault('ShowDetailedTables',False)
    kwargs.setdefault('PdgIdCategorisation',False)
    kwargs.setdefault('StatisticEtaBins',[eta/10. for eta in range(5, 40, 5)])

def ActsTrackParticleTruthDecorationAlgCfg(flags,
                                           name: str = 'ActsTrackParticleTruthDecorationAlg',
                                           **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TrackToTruthAssociationMaps','ActsCombinedTracksToTruthParticleAssociation')
    kwargs.setdefault('TrackParticleContainerName','ActsCombinedTracksParticlesAlt')
    kwargs.setdefault('TruthParticleHitCounts','TruthParticleHitCounts')
    # weights used for computing the matching probability and identifying the best match
    setDefaultTruthMatchingArgs(kwargs)
    kwargs.setdefault('ComputeTrackRecoEfficiency',False)

    if 'TruthSelectionTool' not in kwargs:
        # should be as tight or looser as the TruthSelectionTool when analysing the truth matches
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
        kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
            InDetRttTruthSelectionToolCfg(flags,
                                          name='RelaxedInDetRttTruthSelectionTool',
                                          requireOnlyPrimary=False,
                                          minPt=500.,
                                          maxEta=4.5
                                          )))

    acc.addEventAlgo( CompFactory.ActsTrk.TrackParticleTruthDecorationAlg(name=name, **kwargs) )
    return acc

def ActsTrackFindingValidationAlgCfg(flags,
                                     name: str = 'ActsTracksValidationAlg',
                                     **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TruthParticleHitCounts','TruthParticleHitCounts')
    kwargs.setdefault('TrackToTruthAssociationMap','ActsTracksToTruthParticles')
    setDefaultTruthMatchingArgs(kwargs)
    kwargs.setdefault('ComputeTrackRecoEfficiency',True)
    
    if 'TruthSelectionTool' not in kwargs:
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
        kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
            InDetRttTruthSelectionToolCfg(flags)))

    acc.addEventAlgo( CompFactory.ActsTrk.TrackFindingValidationAlg(name=name, **kwargs) )
    return acc
