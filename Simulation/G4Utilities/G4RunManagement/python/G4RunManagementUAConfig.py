# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SyncPrimaryGeneratorActionToolCfg(flags, name='G4UA::SyncPrimaryGeneratorActionTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.SyncPrimaryGeneratorActionTool(name, **kwargs))
    return result

def SyncRunActionToolCfg(flags, name='G4UA::SyncRunActionTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.SyncRunActionTool(name, **kwargs))
    return result

def SyncEventActionToolCfg(flags, name='G4UA::SyncEventActionTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.SyncEventActionTool(name, **kwargs))
    return result
