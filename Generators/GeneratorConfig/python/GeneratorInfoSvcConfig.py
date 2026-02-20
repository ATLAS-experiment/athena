# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GeneratorInfoSvcCfg(flags, name="GeneratorInfoSvc", **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.GeneratorInfoSvc(name, **kwargs), primary=True, create=True)
    return acc