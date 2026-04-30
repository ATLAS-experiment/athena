# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArCellMuxAlgCfg(
        flags,
        name='LArCellMuxAlg',
        **kwargs):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GlobalSim.LArCellMuxAlg(name,**kwargs)
    cfg.addEventAlgo(alg)

    return cfg
    
