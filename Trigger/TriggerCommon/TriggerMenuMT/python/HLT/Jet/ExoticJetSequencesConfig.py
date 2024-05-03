# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger("TriggerMenuMT.HLT.Jet.JetChainSequences")

from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA

from AthenaConfiguration.ComponentFactory import CompFactory
import functools

def jetEJsMenuSequenceGenCfg(flags, jetsIn):
    
    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetEJsHypoToolFromDict

    # Get track sequence name
    sequenceOut  = flags.Trigger.InDetTracking.fullScan.tracks_FTF
    vertices     = flags.Trigger.InDetTracking.fullScan.vertex_jet

    def makejetEJsMenuSequence():

        reco = InEventRecoCA(
            f"EmergingJets_{jetsIn}Reco",
            inputMaker=CompFactory.InputMakerForRoI(
                "IM_EmergingJets",
                RoITool = CompFactory.ViewCreatorInitialROITool(),
                mergeUsingFeature = True
            )
        )

        selAcc = SelectionCA(f"EmergingJets_{jetsIn}")
        selAcc.mergeReco(reco)

        selAcc.addHypoAlgo(
            CompFactory.TrigJetEJsHypoAlg(
                "L2EmergingJets",
                Tracks = sequenceOut,
                PV     = vertices
            )
        )
        return MenuSequenceCA(flags, selAcc, HypoToolGen=trigJetEJsHypoToolFromDict)
    return functools.partial(makejetEJsMenuSequence)

def jetCRVARMenuSequenceGenCfg(flags, jetsIn):

    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetCRVARHypoToolFromDict
    # Get track sequence name
    from ..CommonSequences.FullScanDefs import fs_cells
    cellsin=fs_cells

    def makejetCRVARMenuSequence():
        reco = InEventRecoCA( 
            f"CalRatioVar_{jetsIn}_RecoSequence",
            inputMaker=CompFactory.InputMakerForRoI(
                "IM_CalRatio_HypoOnlyStep",
                RoITool = CompFactory.ViewCreatorInitialROITool(),
                mergeUsingFeature = True
            )
        )

        selAcc = SelectionCA(f"CalRatioVar_{jetsIn}")
        selAcc.mergeReco(reco)
        selAcc.addHypoAlgo(
            CompFactory.TrigJetCRVARHypoAlg(
                "L2CalRatioVar", 
                Cells  = cellsin
            )
        )
        return MenuSequenceCA(flags, selAcc, HypoToolGen=trigJetCRVARHypoToolFromDict)
    return functools.partial(makejetCRVARMenuSequence)


def jetCRMenuSequenceGenCfg(flags, jetsIn):

    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetCRHypoToolFromDict

    # Get track sequence name
    from ..CommonSequences.FullScanDefs import fs_cells, trkFSRoI
    sequenceOut  = flags.Trigger.InDetTracking.fullScan.tracks_FTF
    cellsin=fs_cells
    
    def makejetCRMenuSequence():
        from .JetMenuSequencesConfig import getTrackingInputMaker
        from .JetTrackingConfig import JetFSTrackingCfg
        trk_acc = JetFSTrackingCfg(flags, trkopt='ftf', RoIs=trkFSRoI)

        reco = InEventRecoCA(f"CalRatio_{jetsIn}Reco", inputMaker=getTrackingInputMaker(flags,'ftf'))
        reco.mergeReco(trk_acc)

        selAcc = SelectionCA(f"CalRatio_{jetsIn}")
        selAcc.mergeReco(reco)
        selAcc.addHypoAlgo(
            CompFactory.TrigJetCRHypoAlg(
                "L2CalRatio",
                Tracks = sequenceOut,
                Cells  = cellsin
            )
        )
        return MenuSequenceCA(flags, selAcc, HypoToolGen=trigJetCRHypoToolFromDict)
    
    return functools.partial(makejetCRMenuSequence)

