#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags, name="GlobalSimPU1Suppression", OutputLevel=DEBUG, dump=False):
    logger.setLevel(OutputLevel)
    
    cfg = ComponentAccumulator()

    hypoTool = CompFactory.GlobalSim.PU1SuppAlgTool('PU1SuppAlgTool')
    hypoTool.OutputLevel = OutputLevel

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name+'Alg')
    alg.globalsim_algs = [hypoTool]

    cfg.addEventAlgo(alg)

    return cfg


