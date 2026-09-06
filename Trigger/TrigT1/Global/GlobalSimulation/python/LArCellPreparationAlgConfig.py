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

    kwargs.setdefault("CaloCellsKey", "AllCalo" if flags.Input.isMC else "SeedLessFS")
    alg = CompFactory.GlobalSim.LArCellPreparationAlg(name,**kwargs)
    if alg.CaloCellsKey ==  "AllCalo":
        from AthenaConfiguration.Enums import Format
        if flags.Input.Format==Format.POOL and 'AllCalo' not in flags.Input.Collections:
            from CaloRec.CaloCellMakerConfig import CaloCellMakerCfg
            cfg.merge(CaloCellMakerCfg(flags,addToOutputStream=False))

    else:
        # ensure we are producing this cell collection ...
        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        cfg.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    cfg.addEventAlgo(alg)

    return cfg
