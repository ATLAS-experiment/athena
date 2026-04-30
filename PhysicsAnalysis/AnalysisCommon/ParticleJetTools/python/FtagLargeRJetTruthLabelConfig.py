"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

Configuration for the FtagLargeRJetTruthLabelTool.

This can be used standalone or called from FtagBaseContent.py.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FtagLargeRJetTruthLabelCfg(flags, jetCollection="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"):
    """Schedule the FtagLargeRJetTruthLabelTool for a given jet collection.

    Decorates each jet with {jetCollection}.FtagLargeRTruthLabel.
    The decoration name is fixed by the C++ default in WriteDecorHandleKey.
    """
    acc = ComponentAccumulator()

    if not flags.Input.isMC:
        return acc

    tool = CompFactory.FtagLargeRJetTruthLabelTool(
        f"FtagLargeRTruthLabel_{jetCollection}",
        JetContainer=jetCollection,
    )

    alg = CompFactory.JetDecorationAlg(
        f"FtagLargeRTruthLabelAlg_{jetCollection}",
        JetContainer=jetCollection,
        Decorators=[tool],
    )

    acc.addEventAlgo(alg)
    return acc
