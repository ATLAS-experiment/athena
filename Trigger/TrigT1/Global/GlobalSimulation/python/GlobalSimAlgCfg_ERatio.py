# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG
logger.setLevel(DEBUG)

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimEratioAlg",
                           OutputLevel=DEBUG,
                           dump=False):
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)
    eratioTool =  CompFactory.GlobalSim.ERatioAlgTool(name+'AlgTool')
    eratioTool.enableDump = dump
    eratioTool.OutputLevel = OutputLevel
    
    alg.globalsim_algs = [eratioTool]
    alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
