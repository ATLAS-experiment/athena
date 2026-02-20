# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimHypoMult",
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()

    # tool1 =  CompFactory.GlobalSim.eFexCvtrAlgTool('eFexCvtrAlgTool')
    tool1 =  CompFactory.GlobalSim.eFexCvtrAlgTool('eFexCvtrAlgTool')
    tool1.OutputLevel = OutputLevel

    tool2 =  CompFactory.GlobalSim.eEmMultTestBench('eEmMultTestBench')
    tool2.rhad = '0'
    tool2.rhad_op = '<='
    tool2.reta = '0'
    tool2.reta_op = '<='
    tool2.wstot = '0'
    tool2.wstot_op = '<='
    
    tool2.OutputLevel = OutputLevel

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name + 'Alg')
    alg.globalsim_algs = [tool1]
    alg.TIPwriters = [tool2]
    alg.enableDumps = dump
    alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    
    return cfg
