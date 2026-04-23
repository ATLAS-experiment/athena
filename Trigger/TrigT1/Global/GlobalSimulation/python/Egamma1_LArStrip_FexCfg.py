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
        caloCellProducer="EMBE1CellsFromCaloCells",
        dump=False,
        dumpTerse=False,
        makeCaloCellContainerChecks=True,
        OutputLevel=DEBUG):
    
    cfg = ComponentAccumulator()


    alg = CompFactory.GlobalSim.Egamma1_LArStrip_Fex(name)

    if caloCellProducer == "EMBE1CellsFromCaloCells":
        caloCellProducer = CompFactory.GlobalSim.EMBE1CellsFromCaloCells()
        caloCellProducer.makeCaloCellContainerChecks = makeCaloCellContainerChecks
        if flags.Input.isMC:
            caloCellProducer.caloCells = "AllCalo"
        else:
            caloCellProducer.caloCells = "SeedLessFS"
    else:
        logger.debug("Cell fetcher " + caloCellProducer + " not supported")
        return cfg

    roiAlgTool = CompFactory.GlobalSim.eFexRoIAlgTool()
    roiAlgTool.etMin = 5000.
    roiAlgTool.etaMin = 0.2
    roiAlgTool.etaMax = 1.4
    
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
    
