# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimEgamma1BDTAlg",
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()

    bdtTool =  CompFactory.GlobalSim.Egamma1BDTAlgTool(name+'AlgTool')
    bdtTool.enableDump = dump
    bdtTool.OutputLevel = OutputLevel

    hypoMultTool =  CompFactory.GlobalSim.eEmMultAlgTool('eEmMultAlgTool')
    hypoMultTool.rhad = '0'
    hypoMultTool.rhad_op = '<='
    hypoMultTool.reta = '0'
    hypoMultTool.reta_op = '<='
    hypoMultTool.wstot = '0'
    hypoMultTool.wstot_op = '<='
    
    hypoMultTool.OutputLevel = OutputLevel

    hypoMultTool.eEmTOBs = "BDTResult"

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)
    alg.globalsim_algs = [bdtTool]
    alg.TIPwriters = [hypoMultTool]
    alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
