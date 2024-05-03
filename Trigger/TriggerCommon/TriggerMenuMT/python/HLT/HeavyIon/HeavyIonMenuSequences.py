# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def egammaFSHIEventShapeMakerCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg
    from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

    acc = ComponentAccumulator()    
    cellMakerAcc = hltCaloCellMakerCfg(flags, 'HLTCaloCellMakerEGFS', 
        roisKey = mapThresholdToL1RoICollection('FSNOSEED'), 
        CellsName = 'CaloCellsEGFS', monitorCells=False)

    acc.merge(cellMakerAcc)
    # configure HI Event Shape by hand,
    # in future need to reuse one from HIGlobal.HIGlobalConfig (will need more args & dependency on flags)
    eventShapeFiller = CompFactory.HIEventShapeFillerTool( 
                EventShapeMapTool = CompFactory.HIEventShapeMapTool())

    eventShapeMakerAlg = CompFactory.HIEventShapeMaker('HLTEventShapeMakerEG',        
        HIEventShapeFillerTool = eventShapeFiller,
        InputCellKey=cellMakerAcc.getPrimary().CellsName,
        NaviTowerKey="",
        InputTowerKey="",
        OutputContainerKey=getTrigEgammaKeys(ion=True).egEventShape) 

    acc.addEventAlgo(eventShapeMakerAlg)
    return acc



def HIFwdGapMenuSequenceGenCfg(flags):
  from ..Config.MenuComponents import InEventRecoCA, SelectionCA, MenuSequenceCA
  from AthenaConfiguration.ComponentFactory import CompFactory
  from TrigHIHypo.TrigHIFwdGapHypoConfig import TrigHIFwdGapHypoToolFromDict

  recoAcc = InEventRecoCA("HIFwdGapReco")
  recoAcc.mergeReco(egammaFSHIEventShapeMakerCfg(flags))

  selAcc = SelectionCA("HLFwdGapSel")
  selAcc.mergeReco(recoAcc)
  selAcc.addHypoAlgo(CompFactory.TrigHIFwdGapHypoAlg())

  return MenuSequenceCA(flags, selAcc, HypoToolGen = TrigHIFwdGapHypoToolFromDict)
