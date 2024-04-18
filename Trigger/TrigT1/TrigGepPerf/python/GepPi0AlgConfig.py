# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepPi0AlgCfg(
        flags,
        name,
        caloCellsProducer="EMB1CellsFromCaloClusters",
        dump=False,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GepPi0Alg(name)
    if caloCellsProducer == "EMB1CellsFromCaloCells":
        alg.caloCellsProducer = CompFactory.EMB1CellsFromCaloCells()
    else:
        alg.caloCellsProducer = CompFactory.EMB1CellsFromCaloClusters()

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    alg.dump = dump
    cfg.addEventAlgo(alg)

    return cfg
    
                     
