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

    if 'TrackToTrackParticleCnvTool' not in kwargs:
        from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvToolCfg
        kwargs.setdefault("TrackToTrackParticleCnvTool", acc.popToolsAndMerge(
            ActsTrackToTrackParticleCnvToolCfg(flags)))

    kwargs.setdefault('RefittedTracksLocation', 'ActsRefittedGSFTracks')
    kwargs.setdefault("SelectedTrackParticleContainerName",
                      flags.Egamma.Keys.Output.TrkPartContainerName)
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")
    kwargs.setdefault("TrackParticlesOutKey", "GSFTrackParticles")

    alg = CompFactory.ActsEMBremCollectionBuilder(name, **kwargs)
    acc.addEventAlgo(alg)

    if flags.Tracking.doTruth and flags.Egamma.doTruthAssociation:
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

  if 'TrackToTrackParticleCnvTool' not in kwargs:
      from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvToolCfg
      kwargs.setdefault("TrackToTrackParticleCnvTool", acc.popToolsAndMerge(
          ActsTrackToTrackParticleCnvToolCfg(flags)))

  kwargs.setdefault('RefittedTracksLocation', 'HLT_IDTrack_Electron_GSFTracks')
  kwargs.setdefault("SelectedTrackParticleContainerName",
                    flags.Tracking.ActiveConfig.tracks_IDTrig)
  kwargs.setdefault("TrackParticleContainerName",
                    flags.Tracking.ActiveConfig.tracks_IDTrig)
  kwargs.setdefault("TrackParticlesOutKey", tpName)

  alg = CompFactory.ActsEMBremCollectionBuilder(name, **kwargs)
  acc.addEventAlgo(alg)

  return acc
