# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from ..CommonSequences.FullScanInDetConfig import commonInDetFullScanCfg,commonInDetLRTCfg
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# ---------------------

# This produces a menu sequence for a step that runs FS FTF tracking
# No selection is applied -- all rejection comes from subsequent steps
def getFullScanRecoOnlySequence(flags):

    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    selAcc = SelectionCA("UncTrkrecoSel")

    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    reco = InEventRecoCA("UncTrkreco",inputMaker=getTrackingInputMaker(flags, "ftf"))
    reco.mergeReco( commonInDetFullScanCfg(flags) )
    selAcc.mergeReco( reco )
    
    HypoAlg = CompFactory.TrigStreamerHypoAlg("UncTrkDummyStream")
    selAcc.addHypoAlgo(HypoAlg)

    log.debug("Building the menu sequence for FullScanRecoOnlySequence")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = StreamerHypoToolGenerator)


# This produces a ComponentAccumulator that can be incorporated into
# an InEventRecoCA downstream. A plain CA is returned so that it
# can be used in independent steps with more complex reco and different
# InputMakers etc.
def getCommonInDetFullScanLRTCfg(flags, flagsLRT):
    combined_reco = ComponentAccumulator()

    combined_reco.merge( commonInDetFullScanCfg(flags) )
    combined_reco.merge( commonInDetLRTCfg(flags, flagsLRT) )

    return combined_reco
