# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG
logger.setLevel(DEBUG)

def Egamma1_LArStrip_FexCfg(
        flags,
        name='Egamma1_LArStrip_Fex',
        caloCellProducer="EMB1CellsFromCaloCells",
        dump=False,
        dumpTerse=False,
        makeCaloCellContainerChecks=True,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()


    alg = CompFactory.GlobalSim.Egamma1_LArStrip_Fex(name)

    if caloCellProducer == "EMB1CellsFromCaloCells":
        caloCellProducer = CompFactory.GlobalSim.EMB1CellsFromCaloCells()
        caloCellProducer.makeCaloCellContainerChecks = makeCaloCellContainerChecks
    else:
        logger.debug("Cell fetcher " + caloCellProducer + " not supported")
        return cfg

    roiAlgTool = CompFactory.GlobalSim.eFexRoIAlgTool()
            
    alg.caloCellProducer = caloCellProducer
    alg.roiAlgTool = roiAlgTool

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        caloCellProducer.OutputLevel = OutputLevel
        roiAlgTool.OutputLevel = OutputLevel
    
    alg.dump = dump
    alg.dumpTerse = dumpTerse
    cfg.addEventAlgo(alg)

    return cfg
    
