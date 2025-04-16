# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AsgForwardElectronLikelihoodToolCfg(flags, name, **kwargs):
    """Configure the electron charge ID selector tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("usePVContainer", flags.Tracking.doVertexFinding)
    AsgForwardElectronLikelihoodTool = CompFactory.AsgForwardElectronLikelihoodTool
    acc.setPrivateTools(AsgForwardElectronLikelihoodTool(name, **kwargs))
    return acc

