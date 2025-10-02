# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#Based on G4UserActionsConfig.py


def TestActionTimerToolCfg(flags, name='G4UA::TestActionTimerTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.TestActionTimerTool(name,**kwargs))
    return result

def TestActionEHistToolCfg(flags, name='G4UA::TestActionEHistTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.TestActionEHistTool(name,**kwargs))
    return result

def TestActionVPTimerToolCfg(flags, name='G4UA::TestActionVPTimerTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.TestActionVPTimerTool(name,**kwargs))
    return result
