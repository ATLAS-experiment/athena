# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AsgElectronChargeIDSelectorToolCfg(flags, name, **kwargs):
    """Configure the electron charge ID selector tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("usePVContainer", flags.Tracking.doVertexFinding)
    AsgElectronChargeIDSelectorTool = CompFactory.AsgElectronChargeIDSelectorTool
    acc.setPrivateTools(AsgElectronChargeIDSelectorTool(name, **kwargs))
    return acc

