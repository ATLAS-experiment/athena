#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsInterop import UnitConstants

def extractChildKwargs(kwargs: dict,
                       prefix: str) -> dict:
    args={}
    for k,v in kwargs.items() :
        if len(k)>len(prefix) and k[0:len(prefix)]==prefix :
           args[k[len(prefix)]:]=v
    return args

def MapToInDetSimDataWrapCfg(flags,
                             collection_name: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    AddressRemappingSvc = CompFactory.AddressRemappingSvc(
        TypeKeyOverwriteMaps = ["InDetSimDataCollection#%s->InDetSimDataCollectionWrap#%s" % (collection_name, collection_name) ]
        )
    acc.addService(AddressRemappingSvc)
    return acc


def PixelClusterToTruthAssociationCfg(flags,
                                      name: str = 'PixelClusterToTruthAssociationAlg',
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapCfg(flags, 'ITkPixelSDO_Map') )
    kwargs.setdefault('InputTruthParticleLinks','xAODTruthLinks')
    kwargs.setdefault('SimData','ITkPixelSDO_Map')
    kwargs.setdefault('DepositedEnergyMin',300) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements','ITkPixelClusters')
    kwargs.setdefault('AssociationMapOut','ITkPixelClustersToTruthParticles')
    acc.addEventAlgo( CompFactory.ActsTrk.PixelClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def StripClusterToTruthAssociationCfg(flags,
                                      name: str = 'StripClusterToTruthAssociationAlg',
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapCfg(flags, 'ITkStripSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks','xAODTruthLinks')
    kwargs.setdefault('SimData','ITkStripSDO_Map')
    kwargs.setdefault('DepositedEnergyMin',600) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements','ITkStripClusters')
    kwargs.setdefault('AssociationMapOut','ITkStripClustersToTruthParticles')
    acc.addEventAlgo( CompFactory.ActsTrk.StripClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def TrackToTruthAssociationCfg(flags,
                               name: str = 'ActsTracksToTruthAssociationAlg',
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapCfg(flags, 'ITkStripSDO_Map') )
    kwargs.setdefault('ACTSTracksLocation','ActsTracks')
    kwargs.setdefault('PixelClustersToTruthAssociationMap','ITkPixelClustersToTruthParticles')
    kwargs.setdefault('StripClustersToTruthAssociationMap','ITkStripClustersToTruthParticles')
    kwargs.setdefault('AssociationMapOut','ActsTracksToTruthParticles')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV)
    acc.addEventAlgo( CompFactory.ActsTrk.TrackToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def TruthParticleHitCountAlgCfg(flags,
                                name: str = 'TruthParticleHitCountAlg',
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapCfg(flags, 'ITkStripSDO_Map') )
    kwargs.setdefault('PixelClustersToTruthAssociationMap','ITkPixelClustersToTruthParticles')
    kwargs.setdefault('StripClustersToTruthAssociationMap','ITkStripClustersToTruthParticles')
    kwargs.setdefault('TruthParticleHitCountsOut','TruthParticleHitCounts')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV) # @TODO introduce flag and synchronise with TrackToTruthAssociationAlg
    kwargs.setdefault('NHitsMin',4)
    acc.addEventAlgo( CompFactory.ActsTrk.TruthParticleHitCountAlg(name=name, **kwargs) )
    return acc


def ITkTruthAssociationCfg(flags,
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel :
        acc.merge(PixelClusterToTruthAssociationCfg(flags, **extractChildKwargs(kwargs,"PixelClusterToTruthAssociation.") ))
    if flags.Detector.EnableITkStrip :
        acc.merge(StripClusterToTruthAssociationCfg(flags, **extractChildKwargs(kwargs,"StripClusterToTruthAssociation.") ))
    return acc
