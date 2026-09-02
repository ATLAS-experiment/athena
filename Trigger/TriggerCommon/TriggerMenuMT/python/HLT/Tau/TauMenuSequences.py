# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDM import recordable
from TrigInDetConfig.utils import getFlagsForActiveConfig

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


# Check the ATLAS Software Docs for more details about the different sub-sequences, CAs, and details
# about the step configuration.


#================================================================
# CaloMVA sequences
#================================================================
@AccumulatorCache
def tauCaloMVASequenceGenCfg(flags: AthConfigFlags, is_probe_leg: bool = False, jet: str = 'lc') -> MenuSequence:
    '''Calorimeter-only reconstruction and hypothesis (BRT-calibrated pT cut)'''

    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the HLTSeeding RoIs (from the input L1 TOBs)
    if jet == 'lc':
        recoAcc = InViewRecoCA(name='tauCaloMVA', InViewRoIs='CaloMVA_RoIs', isProbe=is_probe_leg)
    else:
        recoAcc = InViewRecoCA(name='tauCaloEM', InViewRoIs='CaloMVA_RoIs', isProbe=is_probe_leg)
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    Objects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
            ('SG::AuxElement', 'StoreGateSvc+EventInfo.actualInteractionsPerCrossing'),
            ('SG::AuxElement', 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing'),
            ('CaloBCIDAverage', 'StoreGateSvc+CaloBCIDAverage')
    }
    if ( not flags.Input.isMC ):
        Objects.add( ('LArDeadOTXFromSC' , 'StoreGateSvc+DeadOTXFromSC' ) )
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects=Objects
    ))


    # Reconstruction tools/algorithms:
    
    if jet == 'lc':
        # Topo-clustering
        from TrigCaloRec.TrigCaloRecConfig import tauTopoClusteringCfg
        recoAcc.mergeReco(tauTopoClusteringCfg(flags, RoIs=RoIs))
        # Create new RoIs with an updated position, based on the central axis of the clusters
        from TrigTauRec.TrigTauRoIToolsConfig import tauCaloRoiUpdaterCfg
        recoAcc.mergeReco(tauCaloRoiUpdaterCfg(flags, inputRoIs=RoIs, clusters='HLT_TopoCaloClustersLC', jet='lc'))
        # Construct the calo-only TauJet (with BRT calibration)
        from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloMVACfg
        recoAcc.mergeReco(trigTauRecMergedCaloMVACfg(flags))
    else:
        # Topo-clustering
        from TrigCaloRec.TrigCaloRecConfig import tauEMTopoClusteringCfg
        recoAcc.mergeReco(tauEMTopoClusteringCfg(flags, RoIs=RoIs))
        # Create new RoIs with an updated position, based on the central axis of the clusters
        from TrigTauRec.TrigTauRoIToolsConfig import tauCaloRoiUpdaterCfg
        recoAcc.mergeReco(tauCaloRoiUpdaterCfg(flags, inputRoIs=RoIs, clusters='HLT_TopoCaloClustersRoI', jet=jet))
        # Construct the calo-only TauJet (with BRT calibration)
        from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloEMCfg
        recoAcc.mergeReco(trigTauRecMergedCaloEMCfg(flags))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first, the rob prefetching alg. second, 
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    
    # Hypothesis:
    # The Hypotools in the Hypo algorithm will execute the BRT-calibrated Tau pT cut
    if jet == 'lc':
        selAcc = SelectionCA('tauCalo', isProbe=is_probe_leg)
        selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)
        selAcc.addHypoAlgo(CompFactory.TrigTauJetHypoAlg('TauCaloMVAHypoAlg', TauJetsKey='HLT_TrigTauRecMerged_CaloMVAOnly'))
    else:
        selAcc = SelectionCA('tauCaloEM', isProbe=is_probe_leg)
        selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)
        selAcc.addHypoAlgo(CompFactory.TrigTauJetHypoAlg('TauCaloEMHypoAlg', TauJetsKey='HLT_TrigTauRecMerged_CaloEMOnly'))

    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauCaloMVAHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauCaloMVAHypoToolFromDict)

    return menuSeq



#================================================================
# Calo + Hits step: HitZ + Calo+Hits preselection
#================================================================
@AccumulatorCache
def tauCaloHitsSequenceGenCfg(orig_flags: AthConfigFlags, seq_name: str, precision_seq_name: str, hitz_config: tuple[str, float] | None = None, is_probe_leg: bool = False, jet: str = 'lc') -> MenuSequence:
    '''Calorimeter+Hits RoI updating and preselection hypothesis'''

    tracking_cfg = f'tauHits{seq_name}'
    next_tracking_cfg = f'tauCore{seq_name}'

    flags = getFlagsForActiveConfig(orig_flags, tracking_cfg, log)

    # Create new RoIs from 'UpdatedCaloRoI', resized to the HitZ RoI before running the HitZ and other inference algorithms
    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
        RoisWriteHandleKey=recordable(flags.Tracking.ActiveConfig.roi),
        InViewRoIs='UpdatedCaloRoI',
        doResize=True,
        RoIEtaWidth=flags.Tracking.ActiveConfig.etaHalfWidth,
        RoIPhiWidth=flags.Tracking.ActiveConfig.phiHalfWidth,
        RoIZedWidth=flags.Tracking.ActiveConfig.zedHalfWidth,
    )


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews from the resized RoIs, based on the 'UpdatedCaloRoI' created in the CaloMVA step
    recoAcc = InViewRecoCA(
        name=f'tauCaloHits_{seq_name}',
        RoITool=newRoITool,
        ViewFallThrough=True,
        RequireParentView=True,
        mergeUsingFeature=True,
        isProbe=is_probe_leg
    )
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
            ('xAOD::TauJetContainer', 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly'),
            ('xAOD::TauTrackContainer', 'StoreGateSvc+HLT_tautrack_dummy'),
        }
    ))


    # Reconstruction tools/algorithms:

    # Construct ID space-points in the View
    from TrigInDetConfig.TrigInDetConfig import trigInDetSPFormationCfg
    recoAcc.mergeReco(trigInDetSPFormationCfg(flags, roisKey=RoIs, signatureName=tracking_cfg))

    # Create the high-level xAOD::TrackParticleValidation container from the SPs
    if flags.Detector.GeometryITk:
        from InDetConfig.InDetPrepRawDataToxAODConfig import ITkPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg
        pixel_cluster_container = 'ITkTrigPixelClusters'
    else:
        from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg
        pixel_cluster_container = 'PixelTrigClusters'

    recoAcc.mergeReco(PixelPrepDataToxAODCfg(
        flags,
        SiClusterContainer=pixel_cluster_container, # From the FTF SP reco
        OutputClusterContainer='PixelClusters', # Output xAOD::TrackParticleValidation container
        WriteNNinformation=False,
        UseTruthInfo=False,
    ))

    # Get the BeamSpot from the conditions database
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    recoAcc.mergeReco(BeamSpotCondAlgCfg(flags))

    # Construct the calo+hits TauJet, and run the HitZ inference algorithms
    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloHitsCfg
    from .TauConfigurationTools import getHitZAlgs, getCaloHitsPreselAlgs
    recoAcc.mergeReco(trigTauRecMergedCaloHitsCfg(
        flags,
        seq_name,
        hitz_algs=getHitZAlgs(flags, f'{precision_seq_name}_{seq_name}', precision_seq_name),
        presel_algs=getCaloHitsPreselAlgs(flags, f'{precision_seq_name}_{seq_name}', precision_seq_name),
        input_rois=RoIs,
    ))


    if hitz_config:
        # Create new RoIs with an updated position, based on the central axis of the clusters
        from TrigTauRec.TrigTauRoIToolsConfig import tauHitZRoiUpdaterCfg
        recoAcc.mergeReco(tauHitZRoiUpdaterCfg(
            flags, 
            inputRoIs=RoIs, 
            outputRoIs=f'UpdatedCaloHits{seq_name}RoI',
            taus='HLT_TrigTauRecMerged_CaloHits',
            hitz_alg=hitz_config[0],
            max_pt=1000e3,
            max_sigma=hitz_config[1],
            tracking_cfg=next_tracking_cfg,
        ))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) second, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauCaloHits_{seq_name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The Hypotools in the Hypo algorithm will execute the BRT-calibrated Tau pT cut
    selAcc.addHypoAlgo(CompFactory.TrigTauJetHypoAlg(f'TauCaloHitsHypoAlg_{seq_name}', TauJetsKey='HLT_TrigTauRecMerged_CaloHits'))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauCaloHitsHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauCaloHitsHypoToolFromDict)

    return menuSeq



#================================================================
# 1st FTF step: FTFCore / FTFLRT
#================================================================

@AccumulatorCache
def tauFTFCoreSequenceGenCfg(orig_flags: AthConfigFlags, calohits_seq_name: str | None = None, do_lrt: bool = False, is_probe_leg: bool = False, jet: str = 'lc') -> MenuSequence:
    '''1st FTF step sequence, for both the tauCore(Hits) and tauLRT RoIs'''

    if do_lrt:
        name = 'LRT'
        tracking_cfg = next_tracking_cfg = 'tauLRT'
        output_rois = 'UpdatedTrackLRTRoI'
    else:
        if jet=='em':
            name = 'Core'+jet
            tracking_cfg = 'tauCore'+jet
            next_tracking_cfg = 'tauIso'+jet
            output_rois = 'UpdatedTrackRoI'+jet
        else:
            name = 'Core'
            tracking_cfg = 'tauCore'
            next_tracking_cfg = 'tauIso'
            output_rois = 'UpdatedTrackRoI'

    if calohits_seq_name:
        tracking_cfg += calohits_seq_name
        next_tracking_cfg += calohits_seq_name

    # Retrieve tracking configuration
    flags = getFlagsForActiveConfig(orig_flags, tracking_cfg, log)

    # Source-dependent configuration
    if calohits_seq_name:
        name += f'_{calohits_seq_name}'
        input_rois = f'UpdatedCaloHits{calohits_seq_name}RoI'
        output_rois = f'{output_rois[:-3]}{calohits_seq_name}RoI'
    else:
        input_rois = 'UpdatedCaloRoI'+jet if jet=='em' else 'UpdatedCaloRoI'


    # Create new RoIs from 'UpdatedCaloRoI' or 'UpdatedCaloHitsRoI, resized to 'tauCore/LRT' 
    # before running the FTF algorithms.
    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
        RoisWriteHandleKey=recordable(flags.Tracking.ActiveConfig.roi),
        InViewRoIs=input_rois,
        doResize=not calohits_seq_name, # Do not resize if we're using the CaloHits RoI, which is already resized to the correct size for the FTF step
        RoIEtaWidth=flags.Tracking.ActiveConfig.etaHalfWidth,
        RoIPhiWidth=flags.Tracking.ActiveConfig.phiHalfWidth,
        RoIZedWidth=flags.Tracking.ActiveConfig.zedHalfWidth,
    )


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews from the resized RoIs, based on the upstream RoI created in the CaloMVA or CaloHits steps
    recoAcc = InViewRecoCA(
        f'tauFastTrack{name}',
        RoITool=newRoITool,
        ViewFallThrough=True,
        RequireParentView=True,
        mergeUsingFeature=True,
        isProbe=is_probe_leg
    )
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
        }
    ))

    
    # Reconstruction tools/algorithms:

    # Fast Track Finder (FTF) sequence (the main point of this step)
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    recoAcc.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=RoIs, signatureName=tracking_cfg))

    # Create new RoIs for the next tracking steps (FTFIso and PrecTrack), based on the found tracks
    TrackCollection = flags.Tracking.ActiveConfig.tracks_FTF
    

    from TrigTauRec.TrigTauRoIToolsConfig import tauTrackRoiUpdaterCfg
    recoAcc.mergeReco(tauTrackRoiUpdaterCfg(
        flags,
        inputRoIs=RoIs,
        outputRoIs=output_rois,
        tracks=TrackCollection,

        # Only pass the next tracking config if it's different from the current one (ActiveConfig),
        tracking_cfg=next_tracking_cfg if next_tracking_cfg != tracking_cfg else None,
    ))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) second, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauFTF{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (online monitoring of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauTrackingHypoAlg(
        f'TauFastTrackHypoAlg_PassBy{name}',
        RoIKey='UpdatedTrackLRTRoI' if do_lrt else '',
        TracksKey=TrackCollection,
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackingHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauTrackingHypoToolFromDict)

    return menuSeq



#================================================================
# 2nd FTF step: FTFIso
#================================================================

@AccumulatorCache
def tauFTFIsoSequenceGenCfg(orig_flags: AthConfigFlags, calohits_seq_name: str | None = None, is_probe_leg: bool = False, jet: str = 'lc'):
    '''2nd FTF step sequence, for the tauIso RoI'''

    name = 'Iso'

    # Retrieve tracking configuration
    if jet =='em':
        name = 'Iso'+jet
        previous_tracking_cfg = 'tauCore'+jet
        tracking_cfg = 'tauIso'+jet
    else:
        previous_tracking_cfg = 'tauCore'
        tracking_cfg = 'tauIso'
    if calohits_seq_name:
        tracking_cfg += calohits_seq_name
        previous_tracking_cfg += calohits_seq_name

    flags = getFlagsForActiveConfig(orig_flags, tracking_cfg, log)


    # Source-dependent configuration
    if calohits_seq_name:
        name += f'_{calohits_seq_name}'
        input_rois = f'UpdatedTrack{calohits_seq_name}RoI'
    else:
        input_rois = 'UpdatedTrackRoI'+jet if jet=='em' else 'UpdatedTrackRoI'


    # Create new RoIs from , resized to 'tauCore/LRT' before running the FTF algorithms
    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
        RoisWriteHandleKey=recordable(flags.Tracking.ActiveConfig.roi),
        InViewRoIs=input_rois,
    )


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the RoIs created in the previous step
    recoAcc = InViewRecoCA(
        f'tauFastTrack{name}', 
        RoITool=newRoITool, 
        RequireParentView=True, 
        ViewFallThrough=True, 
        isProbe=is_probe_leg
    )
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    previous_step_flags = getFlagsForActiveConfig(orig_flags, previous_tracking_cfg, log)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('xAOD::TrackParticleContainer', f'StoreGateSvc+{previous_step_flags.Tracking.ActiveConfig.tracks_FTF}'),
        }
    ))

    
    # Reconstruction tools/algorithms:

    # Fast Track Finder (FTF) sequence (the main point of this step)
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    recoAcc.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=RoIs, signatureName=tracking_cfg))

    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) second, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauFTF{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (debug logging of number of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauTrackingHypoAlg(
        f'TauFastTrackHypoAlg_PassBy{name}',
        TracksKey=flags.Tracking.ActiveConfig.tracks_FTF,
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackingHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauTrackingHypoToolFromDict)

    return menuSeq



#================================================================
# Precision Tracking step
#================================================================

@AccumulatorCache
def tauPrecTrackSequenceGenCfg(orig_flags: AthConfigFlags, calohits_seq_name: str | None = None, do_lrt: bool = False, is_probe_leg: bool = False, jet: str = 'lc') -> MenuSequence:
    '''Precision Tracking step sequence, for both the tauIso and tauLRT RoIs'''

    if do_lrt:
        name = 'LRT'
        tracking_cfg = 'tauLRT'
    elif jet=='em':
        name = 'Iso'+jet
        tracking_cfg = 'tauIso'+jet
    else:
        name = 'Iso'
        tracking_cfg = 'tauIso'

    if calohits_seq_name:
        name += f'_{calohits_seq_name}'
        tracking_cfg += calohits_seq_name
        input_rois = f'tauFastTrack{name}_{calohits_seq_name}'
    else:
        input_rois = f'tauFastTrack{name}'

    # Retrieve tracking configuration
    flags = getFlagsForActiveConfig(orig_flags, tracking_cfg, log)


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the RoIs created in the previous steps (tauIso and tauLRT)
    recoAcc = InViewRecoCA(
        name=f'tauPrecTrack{name}', 
        RoITool=CompFactory.ViewCreatorPreviousROITool(),
        InViewRoIs=input_rois,
        RequireParentView=True,
        ViewFallThrough=True,                           
        isProbe=is_probe_leg,
    )
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('SG::AuxElement', 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing'),
        }
    ))


    # Reconstruction tools/algorithms:

    # Precision Tracking sequence (track extension to the TRT and refitting)
    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    recoAcc.mergeReco(trigInDetPrecisionTrackingCfg(flags, rois=RoIs, signatureName=tracking_cfg))

    # Vertexing sequence
    from TrigInDetConfig.TrigInDetConfig import trigInDetVertexingCfg
    recoAcc.mergeReco(trigInDetVertexingCfg(flags, flags.Tracking.ActiveConfig.tracks_IDTrig, flags.Tracking.ActiveConfig.vertex))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauPT{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (debug logging of number of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauTrackingHypoAlg(
        f'TauPrecTrackHypoAlg_PassBy{name}',
        TracksKey=flags.Tracking.ActiveConfig.tracks_IDTrig, 
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackingHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauTrackingHypoToolFromDict)

    return menuSeq



#================================================================
# Precision Tau step
#================================================================

@AccumulatorCache
def tauPrecisionSequenceGenCfg(orig_flags: AthConfigFlags, seq_name: str, calohits_seq_name: str | None = None, output_name: str | None = None, do_lrt: bool = False, is_probe_leg: bool = False, jet: str = 'lc') -> MenuSequence:
    '''Precision Tau step sequence, for all ID and reconstruction settings'''

    if jet=='em': 
        seq_name="EM"
        output_name='EM'
        orig_seq_name='EM'
    orig_seq_name = seq_name
    if output_name is None: output_name = orig_seq_name

    if do_lrt:
        tracking_cfg = 'tauLRT'
        input_rois = 'tauFastTrackLRT'
    elif jet=='em':
        tracking_cfg = 'tauIso'+jet
        input_rois = 'tauFastTrackIso'+jet
    else:
        tracking_cfg = 'tauIso'
        input_rois = 'tauFastTrackIso'

    if calohits_seq_name:
        seq_name += f'_{calohits_seq_name}'
        tracking_cfg += calohits_seq_name
        input_rois += f'_{calohits_seq_name}'
        input_taus = 'HLT_TrigTauRecMerged_CaloHits'
        input_tau_tracks = 'HLT_tautrack_CaloHits_dummy'
    else:
        if jet=='lc':
            input_taus = 'HLT_TrigTauRecMerged_CaloMVAOnly'
        else:
            input_taus = 'HLT_TrigTauRecMerged_CaloEMOnly'
        input_tau_tracks = 'HLT_tautrack_dummy'

    # Retrieve tracking configuration
    flags = getFlagsForActiveConfig(orig_flags, tracking_cfg, log)


    # Get the list of TauIDs to execute
    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getPrecisionSequenceTauIDs
    tau_ids = getPrecisionSequenceTauIDs(flags, seq_name, orig_seq_name)


    # Get the list of the decorated variables from the previous step taus to be copied to the new precision taus
    decors_to_copy = []
    from .TauConfigurationTools import getHitZAlgs, getHitZVariables, getCaloHitsPreselAlgs, getTauIDScoreVariables
    if calohits_seq_name:
        decors_to_copy += [var for alg in getHitZAlgs(flags, seq_name, orig_seq_name) for var in getHitZVariables(alg)]
        decors_to_copy += [var for alg in getCaloHitsPreselAlgs(flags, seq_name, orig_seq_name) for var in getTauIDScoreVariables(alg)]


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the RoIs created in the previous steps (tauIso and tauLRT)
    recoAcc = InViewRecoCA(
        name=f'tauPrecisionReco_{seq_name}', 
        RoITool=CompFactory.ViewCreatorPreviousROITool(),
        InViewRoIs=input_rois,
        RequireParentView=True,
        ViewFallThrough=True,
        isProbe=is_probe_leg,
    )
    RoIs = recoAcc.inputMaker().InViewRoIs


    # VDV with all the required collections/objects in the View
    # (the VDV checks are disabled unless running with -l DEBUG)
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('SG::AuxElement', 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing'),
            ('xAOD::VertexContainer', f'StoreGateSvc+{flags.Tracking.ActiveConfig.vertex}'),
            ('xAOD::TrackParticleContainer', f'StoreGateSvc+{flags.Tracking.ActiveConfig.tracks_IDTrig}'),
            ('xAOD::TauTrackContainer', f'StoreGateSvc+{input_tau_tracks}'),
            ('xAOD::TauJetContainer', f'StoreGateSvc+{input_taus}'),
        } | {
            ('xAOD::TauJetContainer', f'StoreGateSvc+{input_taus}.{var}')
            for var in decors_to_copy
        }
    ))


    # Reconstruction tools/algorithms:

    # Precision TauJet reconstruction sequence
    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedPrecisionMVACfg
    recoAcc.mergeReco(trigTauRecMergedPrecisionMVACfg(
        flags,
        seq_name,
        tau_ids=tau_ids,
        input_rois=RoIs,
        input_tracks=flags.Tracking.ActiveConfig.tracks_IDTrig,
        input_taus=input_taus,
        input_tau_tracks=input_tau_tracks,
        output_name=output_name,
        decors_to_copy=decors_to_copy,
    ))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauPrecision_{seq_name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The Hypotools in the Hypo algorithm will execute the calibrated Tau pT cut,
    # NTrack cut, NWideTrack cut, and ID WP selections (or meson variable cuts)
    selAcc.addHypoAlgo(CompFactory.TrigTauJetHypoAlg(
        f'TauPrecisionHypoAlg_{seq_name}',
        TauJetsKey=f'HLT_TrigTauRecMerged_{output_name}'
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauPrecisionHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauPrecisionHypoToolFromDict)

    return menuSeq
