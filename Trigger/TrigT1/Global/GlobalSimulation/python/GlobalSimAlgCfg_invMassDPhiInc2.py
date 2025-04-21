# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimHypoContainer",
                           minEt1Cuts = [0,0,0,0],
                           minEt2Cuts = [0,0,0,0],
                           minEta1Cuts = [0,0,0,0],
                           maxEta1Cuts = [0,0,0,0],
                           minEta2Cuts = [0,0,0,0],
                           maxEta2Cuts = [0,0,0,0],
                           minInvMassSqrCuts = [0,0,0,0],
                           maxInvMassSqrCuts = [0,0,0,0],
                           minDeltaPhiCuts = [0,0,0,0],
                           maxDeltaPhiCuts = [0,0,0,0],
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()

    hypoTool =  CompFactory.GlobalSim.InvariantMassDeltaPhiInclusive2AlgTool(
        'InvMassDPhiInc2AlgTool')
    hypoTool.enableDump = dump

    hypoTool.minEt1Cuts = minEt1Cuts
    hypoTool.minEt2Cuts = minEt2Cuts
    
    hypoTool.minEta1Cuts = minEta1Cuts
    hypoTool.maxEta1Cuts = maxEta1Cuts
    
    hypoTool.minEta2Cuts = minEta2Cuts
    hypoTool.maxEta2Cuts = maxEta2Cuts
        
    hypoTool.minInvMassSqrCuts = minInvMassSqrCuts
    hypoTool.maxInvMassSqrCuts = maxInvMassSqrCuts

           
    hypoTool.minDeltaPhiCuts = minDeltaPhiCuts
    hypoTool.maxDeltaPhiCuts = maxDeltaPhiCuts

    hypoTool.OutputLevel = OutputLevel

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name + 'Alg')
    alg.globalsim_algs = [hypoTool]
    alg.enableDumps = dump

    cfg.addEventAlgo(alg)
    
    return cfg
