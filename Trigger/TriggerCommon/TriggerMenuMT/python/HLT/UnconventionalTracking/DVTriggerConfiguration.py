# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging

from TrigInDetConfig.utils import cloneFlagsToActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetLRTCfg

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA, InEventRecoCA

from TrigEDMConfig.TriggerEDM import recordable

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

trkPairOutName = "HLT_TrigDV_VSITrkPair"
vtxOutName = "HLT_TrigDV_VSIVertex"
vtxCountName = "HLT_TrigDV_VtxCount"

def DVRecoFragment(flags):

    selAcc = SelectionCA("DVRecoSequence1")
    
    inputMakerAlg = CompFactory.EventViewCreatorAlgorithm(
        "IMDVRoILRT",
        mergeUsingFeature = False,
        RoITool = CompFactory.ViewCreatorDVROITool(
            'ViewCreatorDVRoI',
            RoisWriteHandleKey  = recordable( flags.Trigger.InDetTracking.DVtxLRT.roi ),
            RoIEtaWidth = flags.Trigger.InDetTracking.DVtxLRT.etaHalfWidth,
            RoIPhiWidth = flags.Trigger.InDetTracking.DVtxLRT.phiHalfWidth,
        ),
        Views = "DVRoIViews",
        InViewRoIs = "InViewRoIs",
        RequireParentView = False,
        ViewFallThrough = True,
        ViewNodeName = selAcc.name+'InView',
    )
    
    reco = InViewRecoCA('DVRecoStep',viewMaker=inputMakerAlg)

    flagsWithTrk = cloneFlagsToActiveConfig(flags, flags.Trigger.InDetTracking.DVtxLRT.input_name, log)

    lrt_algs = trigInDetLRTCfg(
        flagsWithTrk,
        flags.Tracking.ActiveConfig.trkTracks_FTF,
        inputMakerAlg.InViewRoIs,
        in_view=True,
        extra_view_inputs=(
            ( 'xAOD::TrackParticleContainer' , flags.Tracking.ActiveConfig.tracks_FTF ),
            ( 'xAOD::VertexContainer' ,        flags.Tracking.ActiveConfig.vertex ),
        )
    )

    from TrigVrtSecInclusive.TrigVrtSecInclusiveConfig import TrigVrtSecInclusiveCfg
    vertexingAlgs = TrigVrtSecInclusiveCfg( flags, "TrigVrtSecInclusive_TrigDV",
                                            FirstPassTracksName = flags.Tracking.ActiveConfig.tracks_FTF,
                                            SecondPassTracksName = flags.Trigger.InDetTracking.DVtxLRT.tracks_FTF,
                                            PrimaryVertexInputName = flags.Tracking.ActiveConfig.vertex,
                                            VxCandidatesOutputName = recordable(vtxOutName),
                                            TrkPairOutputName = recordable(trkPairOutName) )

    recoAlgSequence = seqAND("DVRecoSeq")
    acc = ComponentAccumulator()

    acc.addSequence(recoAlgSequence)
    
    acc.merge(lrt_algs)
    acc.merge(vertexingAlgs)
    
    reco.mergeReco(acc)
    selAcc.mergeReco(reco)
    return selAcc


def DVRecoSequenceGenCfg(flags):
    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    selAcc = DVRecoFragment(flags)

    HypoAlg = CompFactory.TrigStreamerHypoAlg("TrigDVRecoDummyStream")
    selAcc.addHypoAlgo(HypoAlg)

    log.debug("Building the Step dictinary for TrigDV reco")
    return MenuSequence(flags,
                        selAcc,
                        HypoToolGen = StreamerHypoToolGenerator
                        )




def DVTriggerEDSequenceGenCfg(flags):
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import TrigVSIHypoToolFromDict
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import createTrigVSIHypoAlgCfg

    selAcc = SelectionCA("TrigDVEDEmptyStep")

    theHypoAlg = createTrigVSIHypoAlgCfg(flags, "TrigDVHypoAlg",
                                         verticesKey = recordable(vtxOutName),
                                         vtxCountKey = recordable(vtxCountName))


    #run at the event level
    inputMakerAlg = CompFactory.InputMakerForRoI( "IM_TrigDV_ED" )
    inputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

    reco = InEventRecoCA('DVEDStep',inputMaker=inputMakerAlg)

    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)

    log.info("Building the Step dictinary for DisVtxTrigger!")
    return MenuSequence(flags,
                          selAcc,
                          HypoToolGen = TrigVSIHypoToolFromDict,
                          )
