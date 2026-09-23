#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


def PesistifyTrackParticles(flags,
                            *,
                            trackParticleCollections: list[str]):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    toAOD = []
    # excluded track aux data
    excludedAuxData = ('-clusterAssociation.-TTVA_AMVFVertices_forReco.-AssoClustersUFO'
                       '.-TTVA_AMVFWeights_forReco')
    # remove track decorations used internally by FTAG software
    from InDetConfig.InDetTrackOutputConfig import FTAG_AUXDATA
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE decorations
    from DerivationFrameworkInDet.IDTIDE import IDTIDE_AOD_EXCLUDED_AUXDATA
    excludedAuxData += '.-'.join([''] + IDTIDE_AOD_EXCLUDED_AUXDATA)

    if not flags.Tracking.writeExtendedSi_PRDInfo:
        excludedAuxData += '.-msosLink'

    for collection in trackParticleCollections:
        print(f"* scheduling persistification of track particle collection: {collection}")
        toAOD += [f"xAOD::TrackParticleContainer#{collection}",
                  f"xAOD::TrackParticleAuxContainer#{collection}Aux.{excludedAuxData}"]

    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc


def PersistifyVertexes(flags,
                       *,
                       vertexCollections: list[str]):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    excludedVtxAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV.-TruthEventMatchingInfos.-TruthEventRawMatchingInfos.-VertexMatchType"
    for collection in vertexCollections:
        to_AOD = [
            f"xAOD::VertexContainer#{collection}",
            f"xAOD::VertexAuxContainer#{collection}Aux." + excludedVtxAuxData]

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, to_AOD))
    return acc


def lowPtPassCfg(flags):
    flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                  "Tracking.HeavyIonLowPtPass")

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from InDetConfig.TrackRecoConfig import TrackRecoPassCfg
    ClusterSplitProbContainer = "InDetAmbiguityProcessorSplitProbHeavyIon"
    result, ClusterSplitProbContainer = TrackRecoPassCfg(flags,
                                                         extension="HeavyIonLowPt",
                                                         InputExtendedInDetTracks=["CombinedInDetTracks"],
                                                         doTrackingSiPattern=True,
                                                         ClusterSplitProbContainer=ClusterSplitProbContainer)
    acc.merge(result)
    
    # make track collection with main pass
    TrackContainer = "CombinedInDetTracksWithSecondPass"
    from TrkConfig.TrkTrackCollectionMergerConfig import TrackCollectionMergerAlgCfg
    acc.merge(TrackCollectionMergerAlgCfg(flags,
                                          name="TrackCollectionMergerLowPtAlg",
                                          InputCombinedTracks=['ExtendedTracks', 'ResolvedHeavyIonLowPtTracks'],
#                                          InputCombinedTracks=['ExtendedTracks', 'SiSPSeededHeavyIonLowPtTracks'],
                                          OutputCombinedTracks=TrackContainer,
                                          AssociationMapName=f"PRDtoTrackMapMerge_{TrackContainer}"))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        acc.merge(InDetTrackTruthCfg(flags,
                                     Tracks=TrackContainer,
                                     DetailedTruth=f"{TrackContainer}DetailedTruth",
                                     TracksTruth=f"{TrackContainer}TruthCollection"))

    # create Track particle collection
    TrackParticleContainer = "InDetHeavyIonLowPtTrackParticles"
    from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
    acc.merge(TrackParticleCnvAlgCfg(flags,
                                     name="InDetTrackParticlesLowPtAlg",
                                     TrackContainerName=TrackContainer,
                                     xAODTrackParticlesFromTracksContainerName=TrackParticleContainer,
                                     ClusterSplitProbabilityName="InDetAmbiguityProcessorSplitProbHeavyIonLowPt",
                                     AssociationMapName=f"PRDtoTrackMapMerge_{TrackContainer}"))

    acc.merge(PesistifyTrackParticles(flags,
                                      trackParticleCollections=[TrackParticleContainer]))

    # vertexing
    vertexContainerName = "LowPtPrimaryVertexes"
    from InDetConfig.InDetPriVxFinderConfig import InDetPriVxFinderCfg
    acc.merge(InDetPriVxFinderCfg(flags,
                                  name="LowPtPrimaryVertexAlg",
                                  TracksName=TrackParticleContainer,
                                  VxCandidatesOutputName=vertexContainerName))

    acc.merge(PersistifyVertexes(flags,
                                 vertexCollections=[vertexContainerName]))
    return acc


def InDetHeavyIonSpecialCfg(flags,
                            **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    # Scheduling low pt pass for heavy ion
    acc.merge(lowPtPassCfg(flags))
    return acc
