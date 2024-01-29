# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def VrtSecInclusiveMenuSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    fscfg = getInDetTrigConfig("fullScan")
    lrtcfg = getInDetTrigConfig("fullScanLRT")

    vsivtxname =  "HLT_TrigVSIVertex"
    # Construct the full reco sequence
    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanLRTCfg
    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    reco = InEventRecoCA("UncFSVSIreco",inputMaker=getTrackingInputMaker("ftf"))

    reco.mergeReco( getCommonInDetFullScanLRTCfg(flags) )

    from TrigVrtSecInclusive.TrigVrtSecInclusiveConfig import TrigVrtSecInclusiveCfg
    theVSI = TrigVrtSecInclusiveCfg(flags, "TrigVrtSecInclusive", fscfg.tracks_FTF(), lrtcfg.tracks_FTF(), fscfg.vertex, vsivtxname, "HLT_TrigVSITrkPair",recordTrkPair=False)
    reco.mergeReco(theVSI)

    # Construct the SelectionCA to hold reco + hypo
    selAcc = SelectionCA("UncFSVSISeq")
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import TrigVSIHypoToolFromDict
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import createTrigVSIHypoAlgCfg

    from TrigEDMConfig.TriggerEDM import recordable
    theHypoAlg = createTrigVSIHypoAlgCfg(flags, "TrigVSIHypoAlg",
                                         verticesKey=recordable(vsivtxname),
                                         vtxCountKey = recordable("HLT_TrigVSI_VtxCount"),
                                         isViewBased=False)
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)
    
    log.info("Building the Step dictinary for TrigVSI!")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigVSIHypoToolFromDict)
                          
