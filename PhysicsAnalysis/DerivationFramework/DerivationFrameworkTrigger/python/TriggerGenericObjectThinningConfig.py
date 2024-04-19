# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TriggerGenericThinningCfg(ConfigFlags, name,  **kwargs):
    acc = ComponentAccumulator()
    got = CompFactory.DerivationFramework.TriggerGenericObjectThinningTool
    acc.addPublicTool(got(name, **kwargs),
                      primary = True)
    return acc