#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import recordable

from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory


@AccumulatorCache
def getBJetSequenceCfg(flags, jc_name=None):
    if not jc_name:
        raise ValueError("jet collection name is empty - pass the full HLT jet collection name to getBJetSequenceCfg().")

    prmVtxKey = flags.Trigger.InDetTracking.fullScan.vertex
    outputRoIName = flags.Trigger.InDetTracking.bjet.roi

    jc_key = f'{jc_name}_'
    # Output container names as defined in TriggerEDMRun3
    BTagName = recordable(f'{jc_key}BTagging')

    roiTool = CompFactory.ViewCreatorCentredOnJetWithPVConstraintROITool(
        RoisWriteHandleKey  = recordable( outputRoIName ),
        VertexReadHandleKey = prmVtxKey,
        PrmVtxLink  = prmVtxKey.replace( "HLT_","" ),
        RoIEtaWidth = flags.Trigger.InDetTracking.bjet.etaHalfWidth,
        RoIPhiWidth = flags.Trigger.InDetTracking.bjet.phiHalfWidth,
        RoIZWidth   = flags.Trigger.InDetTracking.bjet.zedHalfWidth,
    )

    # Second stage of Fast Tracking and Precision Tracking
    bJetBtagSequence = InViewRecoCA(f"BTagViews_{jc_name}", RoITool = roiTool,
                                    InViewRoIs = "InViewRoIs",
                                    mergeUsingFeature = True,
                                    RequireParentView = False,
                                    ViewFallThrough = True,
                                    InViewJets = recordable( f'{jc_key}bJets' ),
                                    # BJet specific
                                    PlaceJetInView = True)
    InputMakerAlg = bJetBtagSequence.inputMaker()
    
    from TriggerMenuMT.HLT.Bjet.BjetTrackingConfig import secondStageBjetTrackingCfg
    secondStageAlgs = secondStageBjetTrackingCfg(flags,
                                                 inputRoI=InputMakerAlg.InViewRoIs,
                                                 inputVertex=prmVtxKey,
                                                 inputJets=InputMakerAlg.InViewJets)

    PTTrackParticles = flags.Trigger.InDetTracking.bjet.tracks_IDTrig # Final output xAOD::TrackParticle collection

    from TriggerMenuMT.HLT.Bjet.BjetFlavourTaggingConfig import flavourTaggingCfg
    flavourTaggingAlgs = flavourTaggingCfg(flags,
                                           inputJets=str(InputMakerAlg.InViewJets),
                                           inputVertex=prmVtxKey,
                                           inputTracks=PTTrackParticles,
                                           BTagName=BTagName,
                                           inputMuons=None)
    bJetBtagSequence.mergeReco(secondStageAlgs)
    bJetBtagSequence.mergeReco(flavourTaggingAlgs)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetch = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=InputMakerAlg.name)

    BjetAthSequence = SelectionCA( f"BjetAthSequence_{jc_name}_step2", )
    BjetAthSequence.mergeReco(bJetBtagSequence, robPrefetchCA=robPrefetch)

    hypo = CompFactory.TrigBjetBtagHypoAlg(
        f"TrigBjetBtagHypoAlg_{jc_name}",
        # keys
        BTaggedJetKey = InputMakerAlg.InViewJets,
        BTaggingKey = BTagName,
        TracksKey = PTTrackParticles,
        PrmVtxKey = InputMakerAlg.RoITool.VertexReadHandleKey,
        # links for navigation
        BTaggingLink = BTagName.replace( "HLT_","" ),
        PrmVtxLink = InputMakerAlg.RoITool.PrmVtxLink,
    )
    BjetAthSequence.addHypoAlgo(hypo)

    from TrigBjetHypo.TrigBjetBtagHypoTool import TrigBjetBtagHypoToolFromDict
    return MenuSequenceCA(flags,
                          BjetAthSequence,
                          HypoToolGen = TrigBjetBtagHypoToolFromDict)

