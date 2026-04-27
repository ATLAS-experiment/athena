# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GPUSystemInfoSvcCfg(flags):
    acc = ComponentAccumulator()
    svc = CompFactory.getComp("AthCUDA::GPUSystemInfoSvc")("GPUSystemInfoSvc")
    acc.addService(svc)
    return acc