# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import (parOR)
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging

from TrigEDMConfig.TriggerEDM import recordable
from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetLRTCfg

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

def DJPromptStep(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoPromptToolFromDict


    hypo_alg = CompFactory.DisplacedJetPromptHypoAlg("DJTrigPromptHypoAlg")

    #get the jet tracking config to get the track collection name
    fscfg = getInDetTrigConfig("fullScan")

    hypo_alg.min_trk_pt = 1.0
    hypo_alg.stdTracksKey = fscfg.tracks_FTF()
    hypo_alg.jetContainerKey = recordable("HLT_AntiKt4EMTopoJets_subjesIS")
    hypo_alg.vtxKey = fscfg.vertex_jet
    hypo_alg.countsKey = "DispJetTrigger_Counts"

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

    lrtcfg = getInDetTrigConfig( 'DJetLRT' )
    roiTool = CompFactory.ViewCreatorCentredOnIParticleROITool('ViewCreatorDJRoI', RoisWriteHandleKey = recordable(lrtcfg.roi), RoIEtaWidth = lrtcfg.etaHalfWidth, RoIPhiWidth = lrtcfg.phiHalfWidth, RoIZedWidth=lrtcfg.zedHalfWidth, UseZedPosition=False)

    InViewRoIs = "InViewRoIs"
    reco = InViewRecoCA("IMDJRoIFTF", RoITool = roiTool, mergeUsingFeature = True, 
                        InViewRoIs = InViewRoIs,
                        RequireParentView = False,ViewFallThrough = True)
    
    fscfg = getInDetTrigConfig("fullScan")

    acc = ComponentAccumulator()
    reco_seq = parOR('UncTrkrecoSeqDJTrigDispRecoSeq')
    acc.addSequence(reco_seq)

    flagsWithTrk = getFlagsForActiveConfig(flags, lrtcfg.name, log)

    lrt_algs = trigInDetLRTCfg(flagsWithTrk,
                               fscfg.trkTracks_FTF(),
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

def DJDispStep(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoDispToolFromDict

    hypo_alg = CompFactory.DisplacedJetDispHypoAlg("DJTrigDispHypoAlg")

    lrtcfg = getInDetTrigConfig( 'DJetLRT' )
    fscfg = getInDetTrigConfig("fullScan")

    hypo_alg.lrtTracksKey = lrtcfg.tracks_FTF()
    hypo_alg.vtxKey = fscfg.vertex_jet

    selAcc = DJDispFragment(flags)

    selAcc.addHypoAlgo(hypo_alg)
    
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigDJHypoDispToolFromDict,
                          )
