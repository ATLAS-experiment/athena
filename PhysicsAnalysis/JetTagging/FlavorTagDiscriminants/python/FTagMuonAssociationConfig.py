"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FTagMuonAssociationCfg(
    cfgFlags,
    jetCollection: str,
    muonCollection: str = "Muons",
    outputMuons: str = "FTagMuons",
    doConeMatching: bool = False,
) -> ComponentAccumulator:
    """Perform muon association with jets."""

    del cfgFlags

    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.FTagGhostMuonAssociationAlg(
            f"FTagGhostMuonAssociationAlg{jetCollection}",
            jetContainer=jetCollection,
            muonContainer=muonCollection,
            outMuons=f"{jetCollection}.{outputMuons}",
            doConeMatching=doConeMatching,
        )
    )
    return acc
