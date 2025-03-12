# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsEMBremCollectionBuilderCfg(flags,
                               name="ActsEMBremCollectionBuilder",
                               **kwargs):
    """ Algorithm to refit Acts tracks using Acts GSF and create Acts refitted tracks.
    Followed by TrackParticle creation and truth decoration."""

    acc = ComponentAccumulator()
    if "ActsFitter" not in kwargs:    
        from ActsConfig.ActsGaussianSumFitterConfig import ActsGaussianSumFitterToolCfg
        kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(
            ActsGaussianSumFitterToolCfg(flags, name="ActsGSFTrackFitter")))
        
    
    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )
    kwargs.setdefault('RefittedTracksLocation', 'ActsRefittedGSFTracks')
    
    kwargs.setdefault("SelectedTrackParticleContainerName",
                      "InDetTrackParticles")
    
    alg = CompFactory.ActsEMBremCollectionBuilder(name, **kwargs)
    acc.addEventAlgo(alg)
    
    
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, "ActsGSFTrackParticleCnvAlg",
                                                ACTSTracksLocation=[kwargs['RefittedTracksLocation'],],
                                                TrackParticlesOutKey="GSFTrackParticles"))
    
    
    from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg
    acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                name="ACTSGSFTrackParticleToTruthAssociationAlg",
                                                ACTSTracksLocation=kwargs['RefittedTracksLocation'],
                                                AssociationMapOut="ACTSGSFTrackParticleToTruthParticleAssociation"))

    from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
    acc.merge(ActsTrackParticleTruthDecorationAlgCfg(flags,
                                                     name="ACTSGSFTrackParticleTruthDecorationAlg",
                                                     TrackToTruthAssociationMaps = ["ACTSGSFTrackParticleToTruthParticleAssociation"],
                                                     TrackParticleContainerName = "GSFTrackParticles"
                                                     ))
    
    return acc

