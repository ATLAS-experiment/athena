# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepCellTowerAlgCfg(
        flags,
        name='GepCellTowerAlg',
        outputCellTowerKey='GEPCellTowers',
        gepCellMapKey='GepCells',
        minEt=0.,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GepCellTowerAlg(name,
                                outputCellTowerKey=outputCellTowerKey,
                                gepCellMapKey=gepCellMapKey,
                                minEt=minEt
                                )
    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
    
                     
