# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from TrigEDMConfig.TriggerEDM import recordable
from TrigGenericAlgs.TrigGenericAlgsConfig import TrigEventInfoRecorderAlgCfg
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetTLAHypoToolFromDict

@AccumulatorCache
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
def JetTLAMenuSequenceCfg( flags, jetsIn, attachBtag=True ):
    
    jetsOut = recordable(jetsIn+"_TLA")
    # retrieves the sequence
    recoAcc = JetTLASequenceCfg(flags, jetsIn=jetsIn)
    #  add the hypo
    hypo = CompFactory.TrigJetTLAHypoAlg("TrigJetTLAHypoAlg_"+jetsIn) 
    hypo.AttachBtag = attachBtag  # avoid attaching btag if creating EMTopo Jets with no tracking
    btagJetTool = CompFactory.TrigBtagTLATool("BtagTLATool_"+jetsIn)

    if hypo.AttachBtag:
        btagJetTool.TLAOutputBTaggingCollection = recordable(jetsOut+"_BTagging")

    hypo.BtagJetTool = btagJetTool
    hypo.TLAOutputName = jetsOut

    selAcc = SelectionCA("TrigJetTLAMainSeq_"+jetsIn)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA( flags,
                           selAcc,
                           HypoToolGen = trigJetTLAHypoToolFromDict
                         )
