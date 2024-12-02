# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def CaloCellsHandlerToolCfg(flags, scheme, energyEncoding, truncationFEBs):
    cfg = ComponentAccumulator()
    caloCellsHandler = CompFactory.CaloCellsHandlerTool(
        GEPEnergyEncodingScheme = scheme,
        HardwareStyleEnergyEncoding = energyEncoding,
        TruncationOfOverflowingFEBs = truncationFEBs
    )
    cfg.setPrivateTools(caloCellsHandler)
    return cfg


def GepClusteringAlgCfg(flags, name='GepClusteringAlg',
                        TopoClAlg='CaloWFS',
                        outputCaloClustersKey='GEPWFSClusters',
                        GEPEnergyEncodingScheme = "6-10-4",
                        HardwareStyleEnergyEncoding = True,
                        TruncationOfOverflowingFEBs = True,
                        OutputLevel=None):

    cfg = ComponentAccumulator()

    tool = cfg.popToolsAndMerge(CaloCellsHandlerToolCfg(flags, GEPEnergyEncodingScheme, HardwareStyleEnergyEncoding, TruncationOfOverflowingFEBs))

    alg = CompFactory.GepClusteringAlg(
        name,
        TopoClAlg=TopoClAlg,
        outputCaloClustersKey=outputCaloClustersKey,
        CaloCellHandler=tool
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg
    
                     
