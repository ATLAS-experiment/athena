# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
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
def tauCaloMVAMenuSequenceGenCfg(flags, is_probe_leg=False):
    '''Calorimeter-only reconstruction and hypothesis (BRT-calibrated pT cut)'''

    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the HLTSeeding RoIs (from the input L1 TOBs)
    recoAcc = InViewRecoCA(name='tauCaloMVA', InViewRoIs='CaloMVA_RoIs', isProbe=is_probe_leg)
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

    # Topo-clustering
    from TrigCaloRec.TrigCaloRecConfig import tauTopoClusteringCfg
    recoAcc.mergeReco(tauTopoClusteringCfg(flags, RoIs=RoIs))

    # Create new RoIs with an updated position, based on the central axis of the clusters
    from TrigTauRec.TrigTauRoIToolsConfig import tauCaloRoiUpdaterCfg
    recoAcc.mergeReco(tauCaloRoiUpdaterCfg(flags, inputRoIs=RoIs, clusters='HLT_TopoCaloClustersLC'))

    # Construct the calo-only TauJet (with BRT calibration)
    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloMVACfg
    recoAcc.mergeReco(trigTauRecMergedCaloMVACfg(flags))


    # Calo ROB prefetching, to reduce number of calls to the readout
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo(flags, nameSuffix=f'IM_{recoAcc.name}_probe' if is_probe_leg else f'IM_{recoAcc.name}')


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first, the rob prefetching alg. second, 
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    selAcc = SelectionCA('tauCalo', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)


    # Hypothesis:
    # The Hypotools in the Hypo algorithm will execute the BRT-calibrated Tau pT cut
    selAcc.addHypoAlgo(CompFactory.TrigTauCaloHypoAlg('TauCaloMVAHypoAlg', TauJetsKey='HLT_TrigTauRecMerged_CaloMVAOnly'))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauCaloMVAHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauCaloMVAHypoToolFromDict)

    return menuSeq



#================================================================
# 1st FTF step: FTFCore / FTFLRT
#================================================================

def _ftfCoreSeq(flags, name, is_probe_leg=False):
    '''1st FTF step sequence, for both the tauCore and tauLRT RoIs'''

    if name not in ['Core', 'LRT']:
        raise ValueError('Invalid name')


    # Create new RoIs from 'UpdatedCaloRoI', resized to 'tauCore/LRT' before running the FTF algorithms
    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
        RoisWriteHandleKey=recordable(flags.Tracking.ActiveConfig.roi),
        InViewRoIs='UpdatedCaloRoI',
        doResize=True,
        RoIEtaWidth=flags.Tracking.ActiveConfig.etaHalfWidth,
        RoIPhiWidth=flags.Tracking.ActiveConfig.phiHalfWidth,
        RoIZedWidth=flags.Tracking.ActiveConfig.zedHalfWidth,
    )


    # If we're running on tauCore RoIs, we will optionally  prefetch ROBs for the larger 'tauIso' RoIs ahead of time,
    # to avoid retrieving more detector information again later in the next step
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if doExtraPrefetching := name == 'Core' and ROBPrefetching.TauCoreLargeRoI in flags.Trigger.ROBPrefetchingOptions:
        prefetchRoIUpdater = CompFactory.RoiUpdaterTool(
            useBeamSpot=True,
            NSigma=1.5,
            EtaWidth=flags.Trigger.InDetTracking.tauIso.etaHalfWidth,
            PhiWidth=flags.Trigger.InDetTracking.tauIso.phiHalfWidth,
            ZedWidth=flags.Trigger.InDetTracking.tauIso.zedHalfWidth,
        )

        prefetchRoITool = CompFactory.ViewCreatorExtraPrefetchROITool(
            RoiCreator=newRoITool,
            RoiUpdater=prefetchRoIUpdater,
            ExtraPrefetchRoIsKey=f'{newRoITool.RoisWriteHandleKey}_forPrefetching',
            PrefetchRoIsLinkName='prefetchRoI',
            MergeWithOriginal=True,
        )


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews from the resized RoIs, based on the 'UpdatedCaloRoI' created in the CaloMVA step
    recoAcc = InViewRecoCA(
        f'tauFastTrack{name}',
        RoITool=prefetchRoITool if doExtraPrefetching else newRoITool,
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
    recoAcc.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=RoIs, signatureName=f'tau{name}'))

    # Create new RoIs for the next tracking steps (FTFIso and PrecTrack), based on the found tracks
    TrackCollection = flags.Tracking.ActiveConfig.trkTracks_FTF
    if name == 'Core':
        from TrigTauRec.TrigTauRoIToolsConfig import tauTrackRoiUpdaterCfg
        recoAcc.mergeReco(tauTrackRoiUpdaterCfg(flags, inputRoIs=RoIs, tracks=TrackCollection))
    elif name == 'LRT':
        from TrigTauRec.TrigTauRoIToolsConfig import tauLRTRoiUpdaterCfg
        recoAcc.mergeReco(tauLRTRoiUpdaterCfg(flags, inputRoIs=RoIs, tracks=TrackCollection))


    # ROB prefetching for the Pixel and SCT data
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=f'IM_{recoAcc.name}')
    if doExtraPrefetching:
        robPrefetchAlg.RoILinkName = prefetchRoITool.PrefetchRoIsLinkName
    

    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first, the rob prefetching alg. second, 
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauFTF{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (online monitoring of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauFastTrackHypoAlg(
        f'TauFastTrackHypoAlg_PassBy{name}',
        RoIForIDReadHandleKey='UpdatedTrackLRTRoI' if name == 'LRT' else '',
        FastTracksKey=TrackCollection
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauFastTrackHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauFastTrackHypoToolFromDict)

    return menuSeq


@AccumulatorCache
def tauFTFTauCoreSequenceGenCfg(flags, is_probe_leg=False):
    # Retrieve 'tauCore' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauCore', log)

    return _ftfCoreSeq(newflags, name='Core', is_probe_leg=is_probe_leg)


@AccumulatorCache
def tauFTFTauLRTSequenceGenCfg(flags, is_probe_leg=False):
    # Retrieve 'tauLRT' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauLRT', log)

    return _ftfCoreSeq(newflags, name='LRT', is_probe_leg=is_probe_leg)



#================================================================
# 2nd FTF step: FTFIso
#================================================================

def _ftfTauIsoSeq(flags, name, is_probe_leg=False):
    '''2nd FTF step sequence, for the tauIso RoI'''

    if name not in ['Iso']:
        raise ValueError('Invalid name')


    # Create new RoIs from , resized to 'tauCore/LRT' before running the FTF algorithms
    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
        RoisWriteHandleKey=recordable(flags.Tracking.ActiveConfig.roi),
        InViewRoIs='UpdatedTrackRoI'
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
    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{recoAcc.name}RecoVDV',
        DataObjects={
            ('TrigRoiDescriptorCollection', f'StoreGateSvc+{RoIs}'),
            ('TrackCollection', f'StoreGateSvc+{flags.Trigger.InDetTracking.tauCore.trkTracks_FTF}'),
        }
    ))

    
    # Reconstruction tools/algorithms:

    # Fast Track Finder (FTF) sequence (the main point of this step)
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    recoAcc.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=RoIs, signatureName=f'tau{name}'))


    # ROB prefetching for the Pixel and SCT data
    # Note: if enabled in the config flags, the tauIso RoI would have already been prefetched in the previous step
    #       through the 'extra prefetching' procedure, so we can skip it
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if name == 'Iso' and ROBPrefetching.TauCoreLargeRoI in flags.Trigger.ROBPrefetchingOptions:
        robPrefetchAlg = None
    else:
        from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
        robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=f'IM_{recoAcc.name}')

    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first, the rob prefetching alg. second (if enabled), 
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    selAcc = SelectionCA(f'tauFTF{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (debug logging of number of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauFastTrackHypoAlg(
        f'TauFastTrackHypoAlg_PassBy{name}',
        FastTracksKey=flags.Tracking.ActiveConfig.trkTracks_FTF,
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauFastTrackHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauFastTrackHypoToolFromDict)

    return menuSeq


@AccumulatorCache
def tauFTFTauIsoSequenceGenCfg(flags, is_probe_leg=False):
    # Retrieve 'tauIso' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauIso', log)

    return _ftfTauIsoSeq(newflags, name='Iso', is_probe_leg=is_probe_leg)



#================================================================
# Precision Tracking step
#================================================================

def _precTrackSeq(flags, name, is_probe_leg=False):
    '''Precision Tracking step sequence, for both the tauIso and tauLRT RoIs'''

    if name not in ['Iso', 'LRT']:
        raise ValueError('Invalid name')


    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the RoIs created in the previous steps (tauIso and tauLRT)
    recoAcc = InViewRecoCA(
        name=f'tauPrecTrack{name}', 
        RoITool=CompFactory.ViewCreatorPreviousROITool(),
        InViewRoIs=f'tauFastTrack{name}',
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
    recoAcc.mergeReco(trigInDetPrecisionTrackingCfg(flags, rois=RoIs, signatureName=f'tau{name}'))

    # Vertexing sequence
    from TrigInDetConfig.TrigInDetConfig import trigInDetVertexingCfg
    recoAcc.mergeReco(trigInDetVertexingCfg(flags, flags.Tracking.ActiveConfig.tracks_IDTrig, flags.Tracking.ActiveConfig.vertex))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last
    # Note: no need to prefetch anything from Pixel or SCT, since we already prefetched 
    #       all the necesary information in the previous step
    selAcc = SelectionCA(f'tauPT{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The hypothesis algorithm/tool does not perform any action (debug logging of number of tracks only)
    selAcc.addHypoAlgo(CompFactory.TrigTauPrecTrackHypoAlg(
        f'TauPrecTrackHypoAlg_PassBy{name}',
        TracksKey=flags.Tracking.ActiveConfig.tracks_IDTrig, 
        RoIForIDReadHandleKey='',
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauPrecTrackHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauPrecTrackHypoToolFromDict)

    return menuSeq


@AccumulatorCache
def tauPrecTrackIsoSequenceGenCfg(flags, is_probe_leg=False):
    # Retrieve 'tauIso' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauIso', log)

    return  _precTrackSeq(newflags, name='Iso', is_probe_leg=is_probe_leg)


@AccumulatorCache
def tauPrecTrackLRTSequenceGenCfg(flags, is_probe_leg=False):
    # Retrieve 'tauLRT' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauLRT', log)

    return _precTrackSeq(newflags, name='LRT', is_probe_leg=is_probe_leg)



#================================================================
# Precision Tau step
#================================================================

def _tauPrecisionSeq(flags, name, tau_ids: list[str], output_name=None, is_probe_leg=False):
    '''Precision Tau step sequence, for all ID and reconstruction settings'''

    # 'tauIso' (and its derivatives) for all chains except for trackLRT
    InViewName = 'Iso' if 'LRT' not in name else 'LRT' 

    # Reconstruction sequence CA (parOR), executting all reco algorithms within the View (from the RoI)
    # in parallel whenever possible, according to their data dependencies.
    # Create the EventViews based on the RoIs created in the previous steps (tauIso and tauLRT)
    recoAcc = InViewRecoCA(
        name=f'tauPrecisionReco_{name}', 
        RoITool=CompFactory.ViewCreatorPreviousROITool(),
        InViewRoIs=f'tauFastTrack{InViewName}',
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
            ('xAOD::TauTrackContainer', 'StoreGateSvc+HLT_tautrack_dummy'),
            ('xAOD::TauJetContainer', 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly'),
        }
    ))


    # Reconstruction tools/algorithms:

    # Precision TauJet reconstruction sequence
    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedPrecisionMVACfg
    recoAcc.mergeReco(trigTauRecMergedPrecisionMVACfg(
        flags,
        name,
        tau_ids=tau_ids,
        input_rois=RoIs,
        input_tracks=flags.Tracking.ActiveConfig.tracks_IDTrig,
        output_name=output_name,
    ))


    # Selection sequence CA (seqAND), executing the recoAcc view creation alg. first,
    # the reco CA (with all the reco algs) after, and the Hypo alg. at last (no ROB prefetching)
    selAcc = SelectionCA(f'tauPrecision_{name}', isProbe=is_probe_leg)
    selAcc.mergeReco(recoAcc)


    # Hypothesis:
    # The Hypotools in the Hypo algorithm will execute the calibrated Tau pT cut,
    # NTrack cut, NWideTrack cut, and ID WP selections (or meson variable cuts)
    selAcc.addHypoAlgo(CompFactory.TrigTauPrecisionHypoAlg(
        f'TauPrecisionHypoAlg_{name}',
        TauJetsKey=f'HLT_TrigTauRecMerged_{output_name if output_name else name}'
    ))


    # Menu sequence, connecting everything internally for the step, and configuring the tools for the Hypo alg.
    # based on the partDict for each chain tau leg
    from TrigTauHypo.TrigTauHypoTool import TrigTauPrecisionHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigTauPrecisionHypoToolFromDict)

    return menuSeq


@AccumulatorCache
def tauPrecisionSequenceGenCfg(flags, seq_name, output_name=None, is_probe_leg=False):
    # Retrieve 'tauIso' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauIso', log)

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getPrecisionSequenceTauIDs

    return _tauPrecisionSeq(newflags, seq_name, tau_ids=getPrecisionSequenceTauIDs(flags, seq_name), output_name=output_name, is_probe_leg=is_probe_leg)


@AccumulatorCache
def tauPrecisionLRTSequenceGenCfg(flags, seq_name, output_name=None, is_probe_leg=False):
    # Retrieve 'tauLRT' RoI tracking configuration
    newflags = getFlagsForActiveConfig(flags, 'tauLRT', log)

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getPrecisionSequenceTauIDs

    return _tauPrecisionSeq(newflags, seq_name, tau_ids=getPrecisionSequenceTauIDs(flags, seq_name), output_name=output_name, is_probe_leg=is_probe_leg)
