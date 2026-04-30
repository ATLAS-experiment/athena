# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG
logger.setLevel(DEBUG)

def Egamma1_LArStrip_Fex_RowAwareCfg(
        flags,
        name='Egamma1_LArStrip_Fex_RowAware',
        **kwargs):

    # NB. If you want to set properties on the caloCellProducer do something like:
    # ..,caloCellProducer=CompFactory.GlobalSim.EMBE1CellsFromCaloCells(makeCaloCellContainerChecks=True),..


    cfg = ComponentAccumulator()

    # this alg needs totalNoise conditions ... configure the condalg for that:
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))

    alg = CompFactory.GlobalSim.Egamma1_LArStrip_Fex_RowAware(name,**kwargs)

    if flags.Input.isMC:
        alg.caloCellProducer.caloCells = "AllCalo"
    else:
        alg.caloCellProducer.caloCells = "SeedLessFS"
        # ensure we are producing this cell collection ...
        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        cfg.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))


    cfg.addEventAlgo(alg)

    return cfg
    
