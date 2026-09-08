# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG
logger.setLevel(DEBUG)

def Egamma1_OnlineMapNbhoodCfg(
        flags,
        name='Egamma1_OnlineMapNbhood',
        **kwargs):

    # NB. If you want to set properties on the caloCellProducer do something like:
    # ..,caloCellProducer=CompFactory.GlobalSim.EMBE1CellsFromCaloCells(makeCaloCellContainerChecks=True),..
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GlobalSim.Egamma1_OnlineMapNbhoodAlg(name, **kwargs)

    
    alg.roiAlgTool.etMin = 5000.
    alg.roiAlgTool.etaMin = 0.0
    alg.roiAlgTool.etaMax = 5.0

    cfg.addEventAlgo(alg)

    return cfg
    
