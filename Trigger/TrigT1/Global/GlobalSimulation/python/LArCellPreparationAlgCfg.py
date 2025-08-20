# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArCellPreparationAlgCfg(
        flags,
        name='LArCellPreparationAlg',
        NumberOfEnergyBits = 6,
        ValueLeastSignificantBit = 40,
        ValueGainFactor = 4,
        gblLArCellsKey = "GlobalLArCells",
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GlobalSim.LArCellPreparationAlg(name)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    alg.numberOfEnergyBits = NumberOfEnergyBits
    alg.valueLeastSignificantBit = ValueLeastSignificantBit
    alg.valueGainFactor = ValueGainFactor
    alg.GlobalLArCellsKey = gblLArCellsKey
    cfg.addEventAlgo(alg)

    return cfg
    
