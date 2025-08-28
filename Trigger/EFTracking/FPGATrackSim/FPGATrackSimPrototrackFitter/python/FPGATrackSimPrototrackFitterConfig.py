#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGAPrototrackFitAlgCfg(flags,
                            name: str = "ActsProtoTrackCreationAndFitAlg",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    
    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )  # PrivateToolHandle
        
    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )  # PrivateToolHandle

    if 'ActsFitter' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
        kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(ActsFitterCfg(flags,
                                                                           ReverseFilteringPt=0,
                                                                           OutlierChi2Cut=float('inf'))))
        
    acc.addEventAlgo(CompFactory.FPGATrackSim.FPGATrackSimPrototrackFitterAlg(name,**kwargs),
                     primary=True)
    return acc

def FPGAProtoTrackFitCfg(flags,  name="FPGAPrototrackFitterConfig", stage = '', useRoads=False, **kwargs):

    ACTSProtoTrackChainTrackKey = "ACTSProtoTrackChainTestTracks"
    FPGAPrototracks = f"ActsProtoTracks{stage}FromFPGARoad" if useRoads else f"ActsProtoTracks{stage}FromFPGATrack"
    acc = ComponentAccumulator()
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
    acc.merge(ITkTrackRecoCfg(flags))


    # ProtoTrackChain Track algo
    acc.merge(FPGAPrototrackFitAlgCfg(flags,"FPGATrackSimProtoTackFitAlg",
                                      ACTSTracksLocation=ACTSProtoTrackChainTrackKey,
                                      FPGATrackSimActsProtoTracks=FPGAPrototracks,
                                      **kwargs))
    return acc

def FPGATruthDecorationCfg(flags, FinalProtoTrackChainxAODTracksKey="xAODFPGAPrototracks", **kwargs):
    ################################################################################
    # Track to Truth association and validation
    from ActsConfig.ActsTruthConfig import ActsTruthParticleHitCountAlgCfg, ActsPixelClusterToTruthAssociationAlgCfg,ActsStripClusterToTruthAssociationAlgCfg
    
    ACTSProtoTrackChainTrackKey = "ACTSProtoTrackChainTestTracks"
    FinalProtoTrackChainxAODTracksKey=FinalProtoTrackChainxAODTracksKey

    acc = ComponentAccumulator()
    truthLinkVector="xAODFPGATruthLinks" if flags.Trigger.FPGATrackSim.useFPGATruthTrackMatching else "xAODTruthLinks"
    acc.merge(ActsPixelClusterToTruthAssociationAlgCfg(flags,
                                                       name="ActsFPGAPixelClusterToTruthAssociationAlg",
                                                       InputTruthParticleLinks=truthLinkVector,
                                                       AssociationMapOut="ITkFPGAPixelClustersToTruthParticles",
                                                       Measurements="xAODPixelClustersFromFPGACluster")) 
    
    acc.merge(ActsStripClusterToTruthAssociationAlgCfg(flags,
                                                       name="ActsFPGAStripClusterToTruthAssociationAlg",
                                                       InputTruthParticleLinks=truthLinkVector,
                                                       AssociationMapOut="ITkFPGAStripClustersToTruthParticles",
                                                       Measurements="xAODStripClustersFromFPGACluster"))
    
    
    acc.merge(ActsTruthParticleHitCountAlgCfg(flags,
                                              name="ActsFPGATruthParticleHitCountAlg",
                                              PixelClustersToTruthAssociationMap="ITkFPGAPixelClustersToTruthParticles",
                                              StripClustersToTruthAssociationMap="ITkFPGAStripClustersToTruthParticles",
                                              TruthParticleHitCountsOut="FPGATruthParticleHitCounts"))
    
    
    from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
    acts_tracks=f"{flags.Tracking.ActiveConfig.extension}Tracks" if not flags.Acts.doAmbiguityResolution else f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
    acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                name=f"{acts_tracks}FPGATrackToTruthAssociationAlg",
                                                PixelClustersToTruthAssociationMap="ITkFPGAPixelClustersToTruthParticles",
                                                StripClustersToTruthAssociationMap="ITkFPGAStripClustersToTruthParticles",
                                                ACTSTracksLocation=ACTSProtoTrackChainTrackKey,
                                                AssociationMapOut=acts_tracks+"FPGAToTruthParticleAssociation"))
    
    
    acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                name=f"{acts_tracks}FPGATrackFindingValidationAlg",
                                                TrackToTruthAssociationMap=acts_tracks+"FPGAToTruthParticleAssociation",
                                                TruthParticleHitCounts="FPGATruthParticleHitCounts"
                                                ))
    
    ################################################################################
    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
    prefix = flags.Tracking.ActiveConfig.extension
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, f"{prefix}ResolvedProtoTrackToAltTrackParticleCnvAlg",
                                                ACTSTracksLocation=[ACTSProtoTrackChainTrackKey,],
                                                TrackParticlesOutKey=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))
   
    from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
    acc.merge(ActsTrackParticleTruthDecorationAlgCfg(flags,
                                                    f"{prefix}ActsFPGATrackParticleTruthDecorationAlg",
                                                    TrackToTruthAssociationMaps=[acts_tracks+"FPGAToTruthParticleAssociation"],
                                                    TrackParticleContainerName=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles",
                                                    TruthParticleHitCounts="FPGATruthParticleHitCounts",
                                                    ComputeTrackRecoEfficiency=True))
    return acc
