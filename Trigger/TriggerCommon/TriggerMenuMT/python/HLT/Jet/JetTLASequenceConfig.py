# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InEventRecoCA
from TrigEDMConfig.TriggerEDM import recordable
from TrigGenericAlgs.TrigGenericAlgsConfig import TrigEventInfoRecorderAlgCfg
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetTLAHypoToolFromDict


def JetTLASequenceCfg(flags, jetsIn):

    ## add the InputMaker (event context)    
    tlaJetInputMakerAlg = CompFactory.InputMakerForRoI("IMTLAJets_"+jetsIn)#,RoIsLink="initialRoI")
    tlaJetInputMakerAlg.RoITool = CompFactory.ViewCreatorPreviousROITool()
    tlaJetInputMakerAlg.mergeUsingFeature = True
    
    # configure an instance of TrigEventInfoRecorderAlg
    recoAcc = InEventRecoCA("JetTLARecoSeq_"+jetsIn,inputMaker=tlaJetInputMakerAlg)
    eventInfoRecorderAlgCfg = TrigEventInfoRecorderAlgCfg(flags, name="TrigEventInfoRecorderAlg_TLA",
                                                          decoratePFlowInfo=True,
                                                          decorateEMTopoInfo=True,
                                                          renounceAll=True, # avoid dependencies, just take what is there, so can share alg between EMTopo & PFlow TLA
                                                          trigEventInfoKey=recordable("HLT_TCEventInfo_TLA"),
                                                          primaryVertexInputName="HLT_IDVertex_FS",
                                                         )
    recoAcc.mergeReco(eventInfoRecorderAlgCfg)

    return recoAcc

@AccumulatorCache
def JetTLAMenuSequenceGenCfg(flags, jetsIn):
    
    jetsOut = recordable(jetsIn+"_TLA")
    recoAcc = JetTLASequenceCfg(flags, jetsIn=jetsIn)

    hypo = CompFactory.TrigJetTLAHypoAlg("TrigJetTLAHypoAlg_"+jetsIn) 

    hypo.TLAOutputName = jetsOut

    selAcc = SelectionCA("TrigJetTLAMainSeq_"+jetsIn)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequence( flags,
                           selAcc,
                           HypoToolGen = trigJetTLAHypoToolFromDict
                         )
