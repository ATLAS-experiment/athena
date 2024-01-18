# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from CaloIdentifier import SUBCALO
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory     import CompFactory

def TileCellFilterAlgCfg( flags,
                          CellsName = "AllCalo",
                          OutputCellsName="SelectedCells",
                          MaxNCells=200000,
                          CellSigmaCut=4):

    acc = ComponentAccumulator()

    tileCellFilter = CompFactory.CaloCellFilterAlg \
        (OutputCellsName,
         CaloNums = [SUBCALO.TILE],
         CellsName = CellsName,
         OutputCellsName = OutputCellsName ,
         MaxNCells = MaxNCells,
         CellSigmaCut = CellSigmaCut)
    acc.addEventAlgo (tileCellFilter)

    return acc
