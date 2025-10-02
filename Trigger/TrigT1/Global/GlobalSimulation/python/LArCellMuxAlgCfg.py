# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArCellMuxAlgCfg(
        flags,
        name='LArCellMuxAlg',
        gblLArCellsKey = "GlobalLArCells",
        writeMuxInputBitstreamToFile = True,
        writeMuxOutputBitstreamToFile = True,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GlobalSim.LArCellMuxAlg(name)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    alg.GlobalLArCellsKey = gblLArCellsKey
    alg.WriteMuxInputBitstreamToFile = writeMuxInputBitstreamToFile
    alg.WriteMuxOutputBitstreamToFile = writeMuxOutputBitstreamToFile
    cfg.addEventAlgo(alg)

    return cfg
    
