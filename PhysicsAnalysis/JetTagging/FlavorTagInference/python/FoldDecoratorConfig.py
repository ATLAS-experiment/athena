# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import json

def FoldDecoratorCfg(flags, jetCollection='AntiKt4EMPFlowJets', prefix=''):
    ca = ComponentAccumulator()
    evt_id = 'mcEventNumber' if flags.Input.isMC else 'eventNumber'
    common = dict(
        eventID=f'EventInfo.{evt_id}',
        salt=42,
        jetCollection=jetCollection,
        associations=['constituentLinks', 'GhostTrack'],
        jetVariableSaltSeeds={
            'constituentLinks': 0,
            'GhostTrack': 1,
        },
    )
    # original folding algorithm, used for GN2v01 right now
    ca.addEventAlgo(
        CompFactory.FlavorTagInference.FoldDecoratorAlg(
            f'{prefix}FoldHashWithHits{jetCollection}',
            **common,
            constituentChars=json.dumps({
                'GhostTrack': [
                    'numberOfPixelHits',
                    'numberOfSCTHits',
                ]
            }),
            constituentSaltSeeds={
                'numberOfPixelHits': 2,
                'numberOfSCTHits': 3,
            },
            jetFoldHash=f'{prefix}jetFoldHash',
        )
    )

    # simpler version, doesn't seem as random though
    ca.addEventAlgo(
        CompFactory.FlavorTagInference.FoldDecoratorAlg(
            f'{prefix}FoldHashWithoutHits{jetCollection}',
            **common,
            jetFoldHash=f'{prefix}jetFoldHash_noHits',
        )
    )
    # version using jetRank, which should be unique for each jet in
    # the event
    ca.addEventAlgo(
        CompFactory.FlavorTagInference.FoldDecoratorAlg(
            f'{prefix}FoldHashJetRank{jetCollection}',
            eventID=f'EventInfo.{evt_id}',
            salt=42,
            jetCollection=jetCollection,
            ints=['jetRank'],
            jetVariableSaltSeeds={
                'jetRank': 137,
            },
            jetFoldHash=f'{prefix}jetFoldRankHash',
        )
    )

    return ca
