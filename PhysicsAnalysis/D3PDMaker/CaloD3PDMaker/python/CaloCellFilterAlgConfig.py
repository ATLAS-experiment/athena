# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from CaloIdentifier import SUBCALO
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloCellFilterAlgCfg( flags,
                          CellsName = "AllCalo",
                          OutputCellsName="SelectedCells",
                          MaxNCells=200000,
                          CellSigmaCut=4,
                          CaloNums=[SUBCALO.LAREM, SUBCALO.LARHEC, SUBCALO.LARFCAL, SUBCALO.TILE],
                          CaloSamplings=[],
                          CellEnergyThreshold=0.) :

    acc = ComponentAccumulator()
    noiseType = "totalNoise"

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge (CaloNoiseCondAlgCfg (flags, noiseType))

    caloCellFilter = CompFactory.CaloCellFilterAlg \
        (OutputCellsName,
         CaloNums = CaloNums,
         CaloSamplings = CaloSamplings,
         CellsName = CellsName,
         OutputCellsName = OutputCellsName ,
         MaxNCells = MaxNCells,
         CellSigmaCut = CellSigmaCut,
         CellEnergyThreshold = CellEnergyThreshold,
         CaloNoise = noiseType)
    acc.addEventAlgo (caloCellFilter)

    return acc
