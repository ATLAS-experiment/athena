# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def GepCellsHandlerAlgCfg(flags, name='GepCellsHandlerAlg', 
                           outputGepCellsKey='GepCells', 
                           GEPEnergyEncodingScheme = "6-10-4", 
                           HardwareStyleEnergyEncoding = True, 
                           TruncationOfOverflowingFEBs = True,
                           OutputLevel=None):

    cfg = ComponentAccumulator()

    alg = CompFactory.GepCellsHandlerAlg(
        name,
        outputGepCellsKey=outputGepCellsKey,
        GEPEnergyEncodingScheme = GEPEnergyEncodingScheme,
        HardwareStyleEnergyEncoding = HardwareStyleEnergyEncoding,
        TruncationOfOverflowingFEBs = TruncationOfOverflowingFEBs
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg

