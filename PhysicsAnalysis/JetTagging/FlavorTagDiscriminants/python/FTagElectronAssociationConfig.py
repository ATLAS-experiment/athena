"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FTagElectronAssociationCfg(cfgFlags, jetCollection: str) -> ComponentAccumulator:
    """Perform electrion association with jets.

    See [FTagGhostElectronAssociationAlg.h](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/JetTagging/FlavorTagDiscriminants/FlavorTagDiscriminants/FTagGhostElectronAssociationAlg.h)
    
    Parameters
    ----------
    jetCollection : str
        The name of the jet collection to which the electrons are associated. If not provided, it will be taken from the dumper config.
    """
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.FTagGhostElectronAssociationAlg(
            "FTagGhostElectronAssociationAlg",
            jetContainer=jetCollection,
            outElectrons=f"{jetCollection}.FTagElectrons",
        )
    )
    return acc
        