# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InEventRecoCA
from AthenaCommon.CFElements import seqAND
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator



def UTTJetRecoSequenceGenCfg(flags):

        topoClusterSequence = jetmetTopoClusteringCfg(flags,RoIs = '')
 
        from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

        from ..Jet.JetRecoSequencesConfig  import JetRecoDataDeps, JetRecoCfg
        from ..Jet.JetRecoCommon     import extractRecoDict
        from ..Menu.SignatureDicts   import JetChainParts_Default
        
        jetRecoDict = extractRecoDict([JetChainParts_Default])
        jetRecoDict.update( 
                {'recoAlg': 'a4', 'constitType': 'tc', 'clusterCalib': 'em', 'constitMod': '', 'trkopt': 'notrk'}
        )

        jetDefDict = JetRecoDataDeps(flags, **jetRecoDict)
        JetCA = JetRecoCfg(flags, **jetDefDict)
        HypoAlg = CompFactory.TrigStreamerHypoAlg("UTTJetRecDummyStream")

        from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
        IMalg = clusterFSInputMaker()

        selAcc = SelectionCA('UTTJetRecoSeq')
        reco = InEventRecoCA('UTTJetRecoStep',inputMaker=IMalg)

        acc = ComponentAccumulator()
        jetseq = seqAND('UTTJetPartSeq')
        acc.addSequence(jetseq)
        acc.merge(topoClusterSequence)
        acc.merge(JetCA)

        reco.mergeReco(acc)
        
        selAcc.mergeReco(reco)
        selAcc.addHypoAlgo(HypoAlg)
        
        return MenuSequence(flags,
                              selAcc,
                              HypoToolGen = StreamerHypoToolGenerator
                              )


def HitDVHypoSequenceGenCfg(flags):
        from TrigLongLivedParticlesHypo.TrigHitDVHypoConfig import TrigHitDVHypoToolFromDict
        from TrigLongLivedParticlesHypo.TrigHitDVHypoConfig import TrigHitDVHypoAlgCfg

#                                     Sequence    = seqAND("HitDVEmptyStep",[DummyInputMakerAlg]),

        selAcc = SelectionCA('HitDVSeq')


        theHitDVHypo = TrigHitDVHypoAlgCfg(flags, "HitDV")



        DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_HitDV_HypoOnlyStep" )
        DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
        reco = InEventRecoCA('HitDVEmptyStep',inputMaker=DummyInputMakerAlg)

        selAcc.mergeReco(reco)
        selAcc.mergeHypo(theHitDVHypo)


        return MenuSequence( flags,
                               selAcc,
                               HypoToolGen = TrigHitDVHypoToolFromDict
                              )
