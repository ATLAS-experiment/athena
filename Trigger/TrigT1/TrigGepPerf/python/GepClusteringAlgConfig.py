# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepClusteringAlgCfg(flags, name='GepClusteringAlg',
                        TopoClAlg='CaloWFS',
                        gepCellMapKey='GepCells',
                        outputCaloClustersKey='GEPWFSClusters',
                        OutputLevel=None):

    cfg = ComponentAccumulator()

    alg = CompFactory.GepClusteringAlg(
        name,
        TopoClAlg=TopoClAlg,
        gepCellMapKey=gepCellMapKey,
        outputCaloClustersKey=outputCaloClustersKey
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg
    
                     
