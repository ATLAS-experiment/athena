"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FTagMuonAssociationCfg(
    cfgFlags,
    jetCollection: str,
    muonCollection: str = "Muons",
    outputMuons: str | None = None,
    doConeMatching: bool = False,
) -> ComponentAccumulator:
    """Perform muon association with jets."""

    del cfgFlags

    # ghost associated and cone matched muons are different associations,
    # so they get their own algorithm and their own decoration and can run
    # side by side
    suffix = "ConeMatched" if doConeMatching else ""
    if outputMuons is None:
        outputMuons = f"FTagMuons{suffix}"

    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.FTagGhostMuonAssociationAlg(
            f"FTagGhostMuonAssociationAlg{suffix}{jetCollection}",
            jetContainer=jetCollection,
            muonContainer=muonCollection,
            outMuons=f"{jetCollection}.{outputMuons}",
            doConeMatching=doConeMatching,
        )
    )
    return acc
