# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def dEdxTriggerHypoSequenceGenCfg(flags):
        from TrigLongLivedParticlesHypo.TrigdEdxTrackHypoConfig import TrigdEdxTrackHypoToolFromDict
        from TrigLongLivedParticlesHypo.TrigdEdxTrackHypoConfig import TrigdEdxTrackHypoAlgCfg

        selAcc = SelectionCA('dEdxSeq')

        
        thedEdxTrackHypo = TrigdEdxTrackHypoAlgCfg(flags, "dEdxTrack")
        
        from AthenaConfiguration.ComponentFactory import CompFactory
        DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_dEdxTrack_HypoOnlyStep" )
        DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
        reco = InEventRecoCA('dEdxEmptyStep',inputMaker=DummyInputMakerAlg)
        selAcc.mergeReco(reco)
        selAcc.mergeHypo(thedEdxTrackHypo)

        return MenuSequenceCA( flags,
                               selAcc,
                               HypoToolGen = TrigdEdxTrackHypoToolFromDict
                              )
