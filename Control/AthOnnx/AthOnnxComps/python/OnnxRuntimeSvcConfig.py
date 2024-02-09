# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnnxRuntimeSvcCfg(flags, name="OnnxRuntimeSvc", **kwargs):
    acc = ComponentAccumulator()
    
    acc.addService(CompFactory.AthOnnx.OnnxRuntimeSvc(name, **kwargs), primary=True, create=True)

    return acc
