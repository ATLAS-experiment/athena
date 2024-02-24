# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def KeepCellsCfg(flags, streamName = 'StreamAOD'):

    cfg = ComponentAccumulator()

    samplings = [
        "PreSamplerB",
        "EMB1",
        "EMB2",
        "EMB3",
        "PreSamplerE",
        "EME1",
        "EME2",
        "EME3",
        "HEC0",
        "HEC1",
        "HEC2",
        "HEC3",
        "TileBar0",
        "TileBar1",
        "TileBar2",
        "TileGap1",
        "TileGap2",
        "TileGap3",
        "TileExt0",
        "TileExt1",
        "TileExt2",
        "FCAL0",
        "FCAL1",
        "FCAL2",
    ]
    CaloThinCellsBySamplingAlg = CompFactory.CaloThinCellsBySamplingAlg
    alg = CaloThinCellsBySamplingAlg ('CaloThinCellsBySamplingAlg_All_' + streamName,
                                     StreamName = streamName,
                                     SamplingCellsName = samplings,
                                     Cells = 'AllCalo')
    cfg.addEventAlgo(alg)

    return cfg
