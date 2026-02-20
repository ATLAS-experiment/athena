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

    test_bench =  CompFactory.GlobalSim.eEmMultTestBench("eEmMultTestBench")
    test_bench.OutputLevel = OutputLevel
    cfg.addEventAlgo(test_bench)

    tool =  CompFactory.GlobalSim.eEmMultAlgTool('eEmMultAlgTool')
    tool.rhad = '0'
    tool.rhad_op = '<='
    tool.reta = '0'
    tool.reta_op = '<='
    tool.wstot = '0'
    tool.wstot_op = '<='
    
    tool.OutputLevel = OutputLevel

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name + 'Alg')
    alg.TIPwriters = [tool]
    
    alg.enableDumps = dump
    alg.OutputLevel = OutputLevel
    cfg.addEventAlgo(alg)

    
    comparator_alg = CompFactory.GlobalSim.eEmMultTestComparator(
        'eEmMultTestComparator')
    comparator_alg.OutputLevel = OutputLevel


    cfg.addEventAlgo(comparator_alg)
    
    return cfg
