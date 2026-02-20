# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GepEratioAlgCfg(flags, alg_type, name: str,
                    seedsKey: str,
                    caloCellsMapKey: str = 'GepCells',
                    outputEratioDecorKey: str = 'Eratio',
                    **kwargs):
    acc = ComponentAccumulator()

    alg = alg_type(name, **kwargs)
    
    alg.SeedsKey = seedsKey
    alg.gepCellMapKey = caloCellsMapKey
    alg.OutputEratioDecorKey = outputEratioDecorKey

    acc.addEventAlgo(alg, primary=True)

    return acc


def GepEMEratioAlgCfg(flags, name: str,
                    seedsKey: str = 'L1_eEMRoI',
                    caloCellsMapKey='GepCells',
                    outputEratioDecorKey='Eratio',
                    **kwargs):
    return GepEratioAlgCfg(flags, CompFactory.GepEMEratioAlg, name, seedsKey, caloCellsMapKey, outputEratioDecorKey, **kwargs)

def GepTauEratioAlgCfg(flags, name: str,
                    seedsKey: str = 'L1_eTauRoI',
                    caloCellsMapKey='GepCells',
                    outputEratioDecorKey='Eratio',
                    **kwargs):
    return GepEratioAlgCfg(flags, CompFactory.GepTauEratioAlg, name, seedsKey, caloCellsMapKey, outputEratioDecorKey, **kwargs)