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
            acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
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



def TrigActsEMBremCollectionBuilderCfg(flags,
                                       name="TrigActsEMBremCollectionBuilder",
                                       **kwargs):

  acc = ComponentAccumulator()
  
  tpName = kwargs.pop("TrackParticlesOutKey","GSFTrackParticles")

  if "ActsFitter" not in kwargs:    
    from ActsConfig.ActsGaussianSumFitterConfig import ActsGaussianSumFitterToolCfg
    kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(
        ActsGaussianSumFitterToolCfg(flags, name="ActsGSFTrackFitter")))
        
    
  if 'TrackingGeometryTool' not in kwargs:
      from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
      kwargs.setdefault(
          "TrackingGeometryTool",
          acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
      )

  kwargs.setdefault('RefittedTracksLocation', 'HLT_IDTrack_Electron_GSFTracks')
  kwargs.setdefault("SelectedTrackParticleContainerName",
                    flags.Tracking.ActiveConfig.tracks_IDTrig)
    
  alg = CompFactory.ActsEMBremCollectionBuilder(name, **kwargs)
  acc.addEventAlgo(alg)

  from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
  acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, "ActsGSFTrackParticleCnvAlg"+flags.Tracking.ActiveConfig.input_name,
                                              ACTSTracksLocation=[kwargs['RefittedTracksLocation'],],
                                              TrackParticlesOutKey=tpName))

  return acc
