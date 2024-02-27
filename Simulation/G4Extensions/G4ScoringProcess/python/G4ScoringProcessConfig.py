# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def G4ScoringProcessToolCfg(flags, name="G4ScoringProcessTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4ScoringProcessTool(name, **kwargs))
    return result
