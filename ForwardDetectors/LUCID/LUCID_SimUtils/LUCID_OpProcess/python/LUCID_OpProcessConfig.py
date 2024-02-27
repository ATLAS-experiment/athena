# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LucidPhysicsToolCfg(flags, name="LucidPhysicsTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.LucidPhysicsTool(name, **kwargs))
    return result
