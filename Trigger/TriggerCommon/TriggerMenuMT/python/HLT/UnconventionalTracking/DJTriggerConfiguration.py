# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import (parOR)
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging

from TrigEDMConfig.TriggerEDM import recordable
from TrigInDetConfig.utils import cloneFlagsToActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetLRTCfg

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

def DJPromptStepSequenceGenCfg(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoPromptToolFromDict


    hypo_alg = CompFactory.DisplacedJetPromptHypoAlg(
        "DJTrigPromptHypoAlg",
        min_trk_pt = 1.0,
        stdTracksKey = flags.Tracking.ActiveConfig.tracks_FTF,   #fullScan
        vtxKey = flags.Tracking.ActiveConfig.vertex_jet,         
        jetContainerKey = recordable("HLT_AntiKt4EMTopoJets_subjesIS"),
        countsKey = "DispJetTrigger_Counts",
    )

    #run at the event level
    im_alg = CompFactory.InputMakerForRoI( "IM_DJTRIG_Prompt" )
    im_alg.RoITool = CompFactory.ViewCreatorInitialROITool()

    selAcc = SelectionCA('DJTrigPromptSeq')
    reco = InEventRecoCA('DJTrigPromptStep',inputMaker=im_alg)
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(hypo_alg)

    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigDJHypoPromptToolFromDict,
                          )

def DJDispFragment(flags):

    roiTool = CompFactory.ViewCreatorCentredOnIParticleROITool(
        'ViewCreatorDJRoI', 
        RoisWriteHandleKey = recordable(flags.Trigger.InDetTracking.DJetLRT.roi), 
        RoIEtaWidth        = flags.Trigger.InDetTracking.DJetLRT.etaHalfWidth, 
        RoIPhiWidth        = flags.Trigger.InDetTracking.DJetLRT.phiHalfWidth, 
        RoIZedWidth        = flags.Trigger.InDetTracking.DJetLRT.zedHalfWidth,
        UseZedPosition     = False)

    InViewRoIs = "InViewRoIs"
    
    im_alg = CompFactory.EventViewCreatorAlgorithm("IM_DJRoIFTF", mergeUsingFeature = True, RoITool = roiTool, Views = "DJRoIViews", InViewRoIs = InViewRoIs, 
                                                   RequireParentView = False, ViewFallThrough = True, ViewNodeName="DJRoIInViews")

    reco = InViewRecoCA("DJRoIFTF", im_alg)
    
    acc = ComponentAccumulator()
    reco_seq = parOR('UncTrkrecoSeqDJTrigDispRecoSeq')
    acc.addSequence(reco_seq)

    flagsWithTrk = cloneFlagsToActiveConfig(flags, flags.Trigger.InDetTracking.DJetLRT.input_name)

    lrt_algs = trigInDetLRTCfg(flagsWithTrk,
                               flags.Tracking.ActiveConfig.trkTracks_FTF,
                               InViewRoIs,
                               in_view=True,
                               )

    acc.merge(lrt_algs)
    
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.name)

    reco.mergeReco(acc)
    
    selAcc = SelectionCA('UncTrkrecoSeqDJTrigDisp')
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)
    return selAcc

def DJDispStepSequenceGenCfg(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoDispToolFromDict

    hypo_alg = CompFactory.DisplacedJetDispHypoAlg("DJTrigDispHypoAlg",
                                                   lrtTracksKey = flags.Trigger.InDetTracking.DJetLRT.tracks_FTF,
                                                   vtxKey = flags.Tracking.ActiveConfig.vertex_jet)

    selAcc = DJDispFragment(flags)

    selAcc.addHypoAlgo(hypo_alg)
    
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigDJHypoDispToolFromDict,
                          )
