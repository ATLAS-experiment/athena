# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg


def BTagTrackAugmenterAlgCfg(
        flags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName='PrimaryVertices',
        prefix="btagIp_"):

    acc = ComponentAccumulator()
    name = ('BTagTrackAugmenter').lower() + prefix + PrimaryVertexCollectionName + TrackCollection

    # -- create the track augmenter algorithm
    acc.addEventAlgo(CompFactory.Analysis.BTagTrackAugmenterAlg(
        name=name,
        TrackContainer=TrackCollection,
        PrimaryVertexContainer=PrimaryVertexCollectionName,
        prefix=prefix,
        TrackToVertexIPEstimator=acc.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags, 'TrkToVxIPEstimator') ),
        Extrapolator=acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)),
    ))

    return acc
