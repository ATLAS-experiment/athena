# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimEgamma1eRatio",
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()

    baselineTool =  CompFactory.GlobalSim.Egamma1eRatioAlgTool(name+'AlgTool')
    baselineTool.enableDump = dump
    baselineTool.OutputLevel = OutputLevel
    
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)
    alg.globalsim_algs = [baselineTool]
    alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
