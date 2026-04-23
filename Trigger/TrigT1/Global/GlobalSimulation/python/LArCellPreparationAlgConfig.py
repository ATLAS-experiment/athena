# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArCellPreparationAlgCfg(
        flags,
        name='LArCellPreparationAlg',
        **kwargs):
    
    cfg = ComponentAccumulator()

    # this alg needs totalNoise conditions ... configure the condalg for that:
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))

    alg = CompFactory.GlobalSim.LArCellPreparationAlg(name,**kwargs)
    if flags.Input.isMC:
        alg.caloCells = "AllCalo"
    else:
        alg.caloCells = "SeedLessFS"
        # ensure we are producing this cell collection ...
        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        cfg.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    cfg.addEventAlgo(alg)

    return cfg
