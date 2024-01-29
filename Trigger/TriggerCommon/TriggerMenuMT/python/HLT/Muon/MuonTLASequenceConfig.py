#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigEDMConfig.TriggerEDM import recordable
from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFMSonlyHypoToolFromDict
from TrigMuonHypo.TrigMuonHypoMonitoring import TrigMuonTLAHypoMonitoring
from .MuonRecoSequences import muonNames

def getMuonCollections (chainPart):
    muNames = muonNames().getNames('RoI')
    muonName = muNames.EFCBName
    if 'msonly' in chainPart['msonlyInfo']:
        muonName = muNames.EFSAName

    return muonName

@AccumulatorCache
def MuonTLASequenceCfg(flags, muons):
    
    ## add the InputMaker (event context)    
    tlaMuonInputMakerAlg = CompFactory.InputMakerForRoI("IMTLAMuons"+muons)
    tlaMuonInputMakerAlg.mergeUsingFeature = True
    tlaMuonInputMakerAlg.RoITool = CompFactory.ViewCreatorPreviousROITool()
    recoAcc = InEventRecoCA("MuonTLARecoSeq_"+ muons,inputMaker=tlaMuonInputMakerAlg)
    
    sequenceOut = recordable(muons+"_TLA")
    #  add the hypo
    hypo = CompFactory.TrigMuonTLAHypoAlg("TrigMuonTLAHypoAlg_"+muons)  
    hypo.TLAOutputName = sequenceOut  
    hypo.MonTool = TrigMuonTLAHypoMonitoring(flags, "TrigMuonTLAHypoAlg/")

    selAcc = SelectionCA("TrigMuonTLAMainSeq_"+muons)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)
    return selAcc

def MuonTLAMenuSequenceCfg( flags, muChainPart):
    muonsIn = getMuonCollections(muChainPart)  
    selAcc=MuonTLASequenceCfg(flags, muons=muonsIn)

    return MenuSequenceCA( flags,
                           selAcc,
                           HypoToolGen = TrigMuonEFMSonlyHypoToolFromDict
                         )
