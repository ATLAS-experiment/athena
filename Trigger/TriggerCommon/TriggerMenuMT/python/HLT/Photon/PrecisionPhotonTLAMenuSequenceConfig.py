#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from TrigEDMConfig.TriggerEDM import recordable
from TrigEgammaHypo.TrigEgammaTLAPhotonHypoTool import TrigEgammaTLAPhotonHypoToolFromDict 


@AccumulatorCache
def PhotonTLASequenceCfg(flags, photonsIn):
    
    ## add the InputMaker (event context)    
    tlaPhotonInputMakerAlg = CompFactory.InputMakerForRoI("IMTLAPhotons", RoIsLink="initialRoI")
    tlaPhotonInputMakerAlg.RoITool =  CompFactory.ViewCreatorPreviousROITool()
    tlaPhotonInputMakerAlg.mergeUsingFeature = True

    recoAcc = InEventRecoCA("PhotonTLARecoSeq_"+photonsIn, inputMaker=tlaPhotonInputMakerAlg)
    
    sequenceOut = recordable(photonsIn+"_TLA")

    return recoAcc, sequenceOut

@AccumulatorCache
def PhotonTLAMenuSequenceGenCfg( flags, photonsIn ):
    
    # retrieves the "reco" sequence which only consists of the InputMaker
    (recoAcc, sequenceOut) = PhotonTLASequenceCfg(flags, photonsIn=photonsIn)

     #  add the hypo
    hypo = CompFactory.TrigEgammaTLAPhotonHypoAlg("TrigPhotonTLAHypoAlg_"+photonsIn) 
    hypo.TLAOutputName = sequenceOut

    selAcc = SelectionCA("TrigPhotonTLAMainSeq_"+photonsIn)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA( flags,
                           selAcc,
                           HypoToolGen = TrigEgammaTLAPhotonHypoToolFromDict
                         )
