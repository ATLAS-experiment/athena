# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepEtaSoftKillerAlgCfg(flags,
        name='GepEtaSoftKillerAlg',
        inputClustersKey='',
        outputClustersKey='',
        OutputLevel=None):

    cfg = ComponentAccumulator()

    alg = CompFactory.GepEtaSoftKillerAlg(name,
        inputClustersKey=inputClustersKey,
        outputClustersKey=outputClustersKey)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg
