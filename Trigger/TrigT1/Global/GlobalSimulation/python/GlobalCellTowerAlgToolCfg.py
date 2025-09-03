# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GlobalCellTowerAlgToolCfg(
        flags,
        name='GlobalCellTowerAlgTool',
        gblLArCellsKey = "GlobalLArCells",
        gblCellTowersKey = "GlobalCellTowers",
        OutputLevel=None):
    
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)

    cellTowerAlgTool = CompFactory.GlobalSim.GlobalCellTowerAlgTool(name)

    if OutputLevel is not None:
        cellTowerAlgTool.OutputLevel = OutputLevel

    cellTowerAlgTool.GlobalLArCellsKey = gblLArCellsKey
    cellTowerAlgTool.GlobalCellTowersKey = gblCellTowersKey

    alg.globalsim_algs = [cellTowerAlgTool]
    #alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
    
