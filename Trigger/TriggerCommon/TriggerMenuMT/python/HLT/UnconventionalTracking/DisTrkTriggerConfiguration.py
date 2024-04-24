# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def DisTrkTriggerHypoSequenceGenCfg(flags):

        from TrigLongLivedParticlesHypo.TrigDisappearingTrackHypoConfig import TrigDisappearingTrackHypoToolFromDict
        from TrigLongLivedParticlesHypo.TrigDisappearingTrackHypoConfig import createTrigDisappearingTrackHypoAlgCfg

        selAcc = SelectionCA('DisTrkSeq')
        theDisTrkHypo = createTrigDisappearingTrackHypoAlgCfg(flags, "DisTrkTrack")


        DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_DisTrkTrack_HypoOnlyStep" )
        DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()


        reco = InEventRecoCA('DisTrkEmptyStep',inputMaker=DummyInputMakerAlg)
        selAcc.mergeReco(reco)
        selAcc.mergeHypo(theDisTrkHypo)
                        
        log.debug("Building the Step dictinary for DisTrk")
        return MenuSequenceCA(flags,
                              selAcc,
                              HypoToolGen = TrigDisappearingTrackHypoToolFromDict,
                              )
