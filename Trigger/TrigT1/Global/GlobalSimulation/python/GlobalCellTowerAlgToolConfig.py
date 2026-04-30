# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GlobalCellTowerAlgToolCfg(
        flags,
        name='GlobalCellTowerAlgTool',
        **kwargs):
    
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)

    cellTowerAlgTool = CompFactory.GlobalSim.GlobalCellTowerAlgTool(name,**kwargs)

    alg.globalsim_algs = [cellTowerAlgTool]
    #alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
    
