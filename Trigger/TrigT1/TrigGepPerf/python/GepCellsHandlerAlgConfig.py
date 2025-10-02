# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def GepCellsHandlerAlgCfg(flags, name='GepCellsHandlerAlg', 
                           outputGepCellsKey='GepCells', 
                           GEPEnergyEncodingScheme = "6-40-4", 
                           HardwareStyleEnergyEncoding = True, 
                           TruncationOfOverflowingFEBs = True,
                           WriteAllCells = False,
                           OutputLevel=None):

    cfg = ComponentAccumulator()

    alg = CompFactory.GepCellsHandlerAlg(
        name,
        outputGepCellsKey=outputGepCellsKey,
        GEPEnergyEncodingScheme = GEPEnergyEncodingScheme,
        HardwareStyleEnergyEncoding = HardwareStyleEnergyEncoding,
        TruncationOfOverflowingFEBs = TruncationOfOverflowingFEBs,
        WriteAllCells = WriteAllCells
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg

