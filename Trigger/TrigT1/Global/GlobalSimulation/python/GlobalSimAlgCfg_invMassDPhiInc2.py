# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimHypoContainer",
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()

    hypoTool =  CompFactory.GlobalSim.InvariantMassDeltaPhiInclusive2AlgTool(
        'InvMassDPhiInc2AlgTool')
    hypoTool.enableDump = dump
    hypoTool.OutputLevel = OutputLevel

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name + 'Alg')
    alg.globalsim_algs = [hypoTool]
    alg.enableDumps = dump

    cfg.addEventAlgo(alg)
    
    return cfg
