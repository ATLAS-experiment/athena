# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from BTagging.BTagConfig import GetTaggerTrainingMap
from BTagging.BTagConfig import BTagAlgsCfg
from JetTagCalibration.JetTagCalibConfig import JetTagCalibCfg

def FTagPEBJetTagConfig(
        flags,
        jet_collection='HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA',
        track_collection='InDetTrackParticles',
        pv_collection='PrimaryVertices',
    ):

    ca = ComponentAccumulator()

    ca.merge(JetTagCalibCfg(flags))

    ca.merge(
        BTagTrackAugmenterAlgCfg(
            flags,
            TrackCollection=track_collection,
            PrimaryVertexCollectionName=pv_collection,
        )
    )

    trainingMap = GetTaggerTrainingMap(flags, jet_collection)

    ca.merge(
        BTagAlgsCfg(
            flags,
            JetCollection=jet_collection,
            nnList=trainingMap,
            trackCollection=track_collection,
            muons=None,
            AddedJetSuffix = ''
    ))

    return ca
