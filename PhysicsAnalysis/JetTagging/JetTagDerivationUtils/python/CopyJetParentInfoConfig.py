# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


_default_jets = 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets'
_default_parents = 'AntiKt10UFOCSSKJets'


def CopyJetParentInfoCfg(flags, jets=None, parents=None):
    if jets is None:
        jets = _default_jets
        parents = _default_parents
    elif jets != _default_jets and parents is None:
        raise ValueError("You must specify the jet parent collection")

    copied_ints = ['jetRank']

    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.JetLinkMatcherAlg(
            f'{parents}To{jets}LinkCopyAlg',
            targetJet=jets,
            sourceJets=[parents],
            linkName='Parent',
            intsToCopy={x:x for x in copied_ints},
        )
    )
    return ca
