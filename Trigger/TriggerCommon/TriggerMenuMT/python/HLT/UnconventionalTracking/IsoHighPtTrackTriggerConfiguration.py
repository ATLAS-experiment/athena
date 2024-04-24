# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

def IsoHPtTrackTriggerHypoSequenceGenCfg(flags):
        from TrigLongLivedParticlesHypo.TrigIsoHPtTrackTriggerHypoTool import TrigIsoHPtTrackTriggerHypoToolFromDict

        # Setup the hypothesis algorithm
        theIsoHPtTrackTriggerHypo = CompFactory.TrigIsoHPtTrackTriggerHypoAlg(
                "L2IsoHPtTrack",
                trackKey = flags.Tracking.ActiveConfig.tracks_FTF
        )

        selAcc = SelectionCA('UncTrkEmptySeq')

        DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_IsoHPtTrack_HypoOnlyStep" )
        DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

        reco = InEventRecoCA('UncTrkEmptyStep',inputMaker=DummyInputMakerAlg)
        selAcc.mergeReco(reco)
        selAcc.addHypoAlgo(theIsoHPtTrackTriggerHypo)
                               
        log.debug("Building the Step dictinary for IsoHPt!")
        return MenuSequenceCA(flags,
                              selAcc,
                              HypoToolGen = TrigIsoHPtTrackTriggerHypoToolFromDict,
                              )

