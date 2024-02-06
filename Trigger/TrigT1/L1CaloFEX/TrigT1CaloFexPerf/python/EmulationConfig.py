# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def emulateSC_Cfg(flags, CellsIn="SeedLessFS"):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
    if flags.Input.isMC: 
        from LArCabling.LArCablingConfig import LArFebRodMappingCfg
        acc.merge(LArFebRodMappingCfg(flags))

    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
    acc.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey = ""))


    #Use SCEmulation tool that randomly samples time histograms to estimate time for low energy and negative cells forming a supercell 
    acc.addEventAlgo(CompFactory.LVL1.SCEmulation(InputCells=CellsIn, OutputSuperCells = "EmulatedSCellNoBCID"))
    
    from LArROD.LArSCSimpleMakerConfig import LArSuperCellBCIDEmAlgCfg

    larSCargs = {}
    larSCargs["SCellContainerIn"] = "EmulatedSCellNoBCID"
    larSCargs["SCellContainerOut"] = flags.Trigger.L1.L1CaloSuperCellContainerName

    # Apply the pedestal correction
    acc.merge(LArSuperCellBCIDEmAlgCfg(flags, **larSCargs))

    # Given this function emulates supercells, we should also configure the supercell alignment Cond alg
    acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())

    return acc
