# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepCellTowerAlgCfg(
        flags,
        name='GelCellTowerAlg',
        outputCellTowerKey='GEPCellTowers',
        gepCellMapKey='GepCells',
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GepCellTowerAlg(name,
                                outputCellTowerKey=outputCellTowerKey,
                                gepCellMapKey=gepCellMapKey
                                )
    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
    
                     
