# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepTCTowerAlgCfg(
        flags,
        name,
        caloClustersKey,
        outputCaloClustersKey,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GepTCTowerAlg(name,
                                caloClustersKey=caloClustersKey,
                                outputCaloClustersKey=outputCaloClustersKey)
    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
    
                     
