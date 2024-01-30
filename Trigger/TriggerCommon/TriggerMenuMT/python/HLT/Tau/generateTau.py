# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDM import recordable

from TrigInDetConfig.utils import getFlagsForActiveConfig
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def _caloSeq(flags, is_probe_leg=False):
    selAcc = SelectionCA('CaloTau', isProbe=is_probe_leg)

    recoAcc = InViewRecoCA(name       = 'tauCaloMVA', 
                           InViewRoIs = 'CaloMVA_RoIs',
                           isProbe    = is_probe_leg)

    recoAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(name=recoAcc.name+'RecoVDV',
                                                              DataObjects={('TrigRoiDescriptorCollection', 'StoreGateSvc+'+recoAcc.inputMaker().InViewRoIs.Path),
                                                                           #( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+HLT_TAURoI'),
                                                                           ('CaloBCIDAverage', 'StoreGateSvc+CaloBCIDAverage'),
                                                                           ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                                                           ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.actualInteractionsPerCrossing'),
                                                                           ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing')}))

    from TrigCaloRec.TrigCaloRecConfig import tauTopoClusteringCfg
    recoAcc.mergeReco(tauTopoClusteringCfg(flags,
                                           RoIs = recoAcc.inputMaker().InViewRoIs))

    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedCaloOnlyMVACfg
    from TrigTauHypo.TrigTauHypoConfig import tauCaloRoiUpdaterCfg

    recoAcc.mergeReco(tauCaloRoiUpdaterCfg(flags,inputRoIs=recoAcc.inputMaker().InViewRoIs,clusters = 'HLT_TopoCaloClustersLC'))

    recoAcc.mergeReco(trigTauRecMergedCaloOnlyMVACfg(flags))    

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix='IM_'+recoAcc.name+'_probe' if is_probe_leg else 'IM_'+recoAcc.name)    
    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)

    hypoAlg = CompFactory.TrigTauCaloHypoAlg("TauL2CaloMVAHypo",
                                                    taujets = "HLT_TrigTauRecMerged_CaloMVAOnly" )
    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigL2TauHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigL2TauHypoToolFromDict,isProbe=is_probe_leg)   
    return (selAcc , menuCA)


@AccumulatorCache
def tauCaloMVAMenuSeq(flags, name, is_probe_leg=False):
    (selAcc , menuCA) = _caloSeq(flags, is_probe_leg)
    return menuCA 


def _ftfCoreSeq(flags,name,is_probe_leg=False):
    selAcc=SelectionCA('tau'+name+'FTF', isProbe=is_probe_leg)

    newRoITool   = CompFactory.ViewCreatorFetchFromViewROITool( 
                                RoisWriteHandleKey = recordable(flags.Tracking.ActiveConfig.roi),
                                InViewRoIs = 'UpdatedCaloRoI')

    if 'LRT' in name:
       newRoITool.doResize                 = True
       newRoITool.RoIEtaWidth              = flags.Tracking.ActiveConfig.etaHalfWidth
       newRoITool.RoIPhiWidth              = flags.Tracking.ActiveConfig.phiHalfWidth
       newRoITool.RoIZedWidth              = flags.Tracking.ActiveConfig.zedHalfWidth
                                                                                                

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching

    extraPrefetching = ROBPrefetching.TauCoreLargeRoI in flags.Trigger.ROBPrefetchingOptions and 'Core' in name
    if extraPrefetching:
      # Add extra RoI to prefetch ROBs for the subsequent tauIso step together with ROBs for tauCore
      prefetchRoIUpdater                   = CompFactory.RoiUpdaterTool()
      prefetchRoIUpdater.useBeamSpot       = True
      prefetchRoIUpdater.NSigma            = 1.5
      prefetchRoIUpdater.EtaWidth          = flags.Trigger.InDetTracking.tauIso.etaHalfWidth
      prefetchRoIUpdater.PhiWidth          = flags.Trigger.InDetTracking.tauIso.phiHalfWidth
      prefetchRoIUpdater.ZedWidth          = flags.Trigger.InDetTracking.tauIso.zedHalfWidth
      prefetchRoITool                      = CompFactory.ViewCreatorExtraPrefetchROITool()
      prefetchRoITool.RoiCreator           = newRoITool
      prefetchRoITool.RoiUpdater           = prefetchRoIUpdater
      prefetchRoITool.ExtraPrefetchRoIsKey = str(newRoITool.RoisWriteHandleKey) + "_forPrefetching"
      prefetchRoITool.PrefetchRoIsLinkName = "prefetchRoI"
      prefetchRoITool.MergeWithOriginal    = True                                                      

    fastInDetReco = InViewRecoCA('tauFastTrack'+name,RoITool           = prefetchRoITool if extraPrefetching else newRoITool,
                                                ViewFallThrough   = True,
                                                RequireParentView = True,
                                                mergeUsingFeature = True,
                                                isProbe           = is_probe_leg)

    robPrefetchAlg = ROBPrefetchingAlgCfg_Si( flags, nameSuffix='IM_'+fastInDetReco.name)
    if extraPrefetching:
      robPrefetchAlg.RoILinkName = str(prefetchRoITool.PrefetchRoIsLinkName) 

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    fastInDetReco.mergeReco(trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName='tau'+name))
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name='VDVFastTau'+name,
        DataObjects={( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                     ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')}) )

    RoIs = fastInDetReco.inputMaker().InViewRoIs
    TrackCollection = flags.Tracking.ActiveConfig.trkTracks_FTF

    from TrigTauHypo.TrigTauHypoConfig import tauTrackRoiUpdaterCfg,tauLRTRoiUpdaterCfg

    if 'LRT' in name:
       fastInDetReco.mergeReco(tauLRTRoiUpdaterCfg(flags,inputRoIs = RoIs,tracks = TrackCollection))
    else:
       fastInDetReco.mergeReco(tauTrackRoiUpdaterCfg(flags,inputRoIs = RoIs,tracks = TrackCollection))
    
    selAcc.mergeReco(fastInDetReco, robPrefetchCA=robPrefetchAlg)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg('TrackPreSelHypoAlg_PassBy'+name,
                                                 RoIForIDReadHandleKey = 'UpdatedTrackLRTRoI' if 'LRT' in name else '',
                                                 trackcollection       = flags.Tracking.ActiveConfig.trkTracks_FTF )
    selAcc.addHypoAlgo(hypoAlg)
    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)


@AccumulatorCache
def tauFTFTauCoreSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauCore',log)

    name='Core'
    (selAcc , menuCA) = _ftfCoreSeq(newflags,name,is_probe_leg)
    return menuCA 


@AccumulatorCache
def tauFTFTauLRTSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauLRT',log)
    name='LRT'
    (selAcc , menuCA) = _ftfCoreSeq(newflags,name,is_probe_leg)
    return menuCA 


def _ftfTauIsoSeq(flags,name,is_probe_leg=False):
    selAcc=SelectionCA('tau'+name+'FTF', isProbe=is_probe_leg)

    newRoITool = CompFactory.ViewCreatorFetchFromViewROITool(
                             RoisWriteHandleKey = recordable(flags.Tracking.ActiveConfig.roi),
                             InViewRoIs = 'UpdatedTrackRoI')

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si

    fastInDetReco = InViewRecoCA('tauFastTrack'+name,RoITool           = newRoITool,
                                                RequireParentView = True,
                                                ViewFallThrough   = True,
                                                isProbe           = is_probe_leg)

    robPrefetchAlg = ROBPrefetchingAlgCfg_Si( flags, nameSuffix='IM_'+fastInDetReco.name)

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    idTracking = trigInDetFastTrackingCfg(flags, roisKey=fastInDetReco.inputMaker().InViewRoIs, signatureName='tau'+name)
    fastInDetReco.mergeReco(idTracking)
    fastInDetReco.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name='VDVFastTau'+name,
        DataObjects={( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(fastInDetReco.inputMaker().InViewRoIs) ),
                     ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly')}) )

    selAcc.mergeReco(fastInDetReco, robPrefetchCA=robPrefetchAlg)
    hypoAlg = CompFactory.TrigTrackPreSelHypoAlg('TrackPreSelHypoAlg_PassBy'+name,
                                                    trackcollection = flags.Tracking.ActiveConfig.trkTracks_FTF )
    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTauTrackHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)


@AccumulatorCache
def tauFTFTauIsoSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauIso',log)
    name = 'Iso'
    (selAcc , menuCA) = _ftfTauIsoSeq(newflags,name,is_probe_leg)
    return menuCA


def _precTrackSeq(flags,name,is_probe_leg=False):
    selAcc=SelectionCA('tau'+name+'Track', isProbe=is_probe_leg)

    recoAcc = InViewRecoCA(name              = 'prec'+name+'Track', 
                           RoITool           = CompFactory.ViewCreatorPreviousROITool(),
                           InViewRoIs        = 'tauFastTrack'+name,
                           RequireParentView = True,
                           ViewFallThrough   = True,                           
                           isProbe           = is_probe_leg)

    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    precTracking = trigInDetPrecisionTrackingCfg(flags, rois=recoAcc.inputMaker().InViewRoIs, signatureName='tau'+name)
    recoAcc.mergeReco(precTracking)

    ViewVerifyTrk =  CompFactory.AthViews.ViewDataVerifier(
        name='VDVPrecTrkTau'+name,
        DataObjects = {( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+%s' % flags.Tracking.ActiveConfig.tracks_FTF ),
                       ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                       ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(recoAcc.inputMaker().InViewRoIs) ),
                       ( 'xAOD::TauTrackContainer' , 'StoreGateSvc+HLT_tautrack_dummy' ),
                       ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly' ),
                       ( 'xAOD::IParticleContainer' , 'StoreGateSvc+%s' % flags.Tracking.ActiveConfig.tracks_FTF ),
                       })

    recoAcc.addRecoAlgo(ViewVerifyTrk)

    precTracks = flags.Tracking.ActiveConfig.tracks_IDTrig

    from TrigInDetConfig.TrigInDetConfig import trigInDetVertexingCfg
    recoAcc.mergeReco(trigInDetVertexingCfg(flags,precTracks,flags.Tracking.ActiveConfig.vertex))

    selAcc.mergeReco(recoAcc)
    hypoAlg = CompFactory.TrigTrkPrecHypoAlg('TrkPrec'+name+'HypoAlg',
                                                    trackparticles = precTracks, 
                                                    RoIForIDReadHandleKey = '' )
    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigTrkPrecHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigTrkPrecHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)


@AccumulatorCache
def tauPrecTrackIsoSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauIso',log)
    name = 'Iso'
    (selAcc , menuCA) = _precTrackSeq(newflags,name,is_probe_leg)
    return menuCA


@AccumulatorCache
def tauPrecTrackLRTSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauLRT',log)
    name = 'LRT'
    (selAcc , menuCA) = _precTrackSeq(newflags,name,is_probe_leg)
    return menuCA


def _tauPrecSeq(flags,name,is_probe_leg=False):
    selAcc=SelectionCA('tauPrec'+name, isProbe=is_probe_leg)

    InViewName = 'Iso' if 'LRT' not in name else 'LRT' 
    recoAcc = InViewRecoCA(name              = 'prec'+name+'Tau', 
                           RoITool           = CompFactory.ViewCreatorPreviousROITool(),
                           InViewRoIs        = 'tauFastTrack'+InViewName,
                           RequireParentView = True,
                           ViewFallThrough   = True,                           
                           isProbe           = is_probe_leg)

    ViewVerifyID =  CompFactory.AthViews.ViewDataVerifier(
        name='VDVPrecTau'+name,
        DataObjects = {( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+{}'.format(recoAcc.inputMaker().InViewRoIs)),
                       ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing'   ),
                       ( 'xAOD::VertexContainer', 'StoreGateSvc+'+flags.Tracking.ActiveConfig.vertex),
                       ( 'xAOD::TauTrackContainer' , 'StoreGateSvc+HLT_tautrack_dummy' ),
                       ( 'xAOD::TauJetContainer' , 'StoreGateSvc+HLT_TrigTauRecMerged_CaloMVAOnly' ),
                       ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+flags.Tracking.ActiveConfig.tracks_IDTrig )})

    recoAcc.addRecoAlgo(ViewVerifyID)

    from TrigTauRec.TrigTauRecConfig import trigTauRecMergedPrecisionMVACfg
    tauPrecisionAlg = trigTauRecMergedPrecisionMVACfg(flags, name, inputRoIs = recoAcc.inputMaker().InViewRoIs, tracks = flags.Tracking.ActiveConfig.tracks_IDTrig)

    recoAcc.mergeReco(tauPrecisionAlg)

    selAcc.mergeReco(recoAcc)
    hypoAlg = CompFactory.TrigEFTauMVHypoAlg('EFTauMVHypoAlg'+name,
                                                    taujetcontainer = 'HLT_TrigTauRecMerged_'+name)
    selAcc.addHypoAlgo(hypoAlg)

    from TrigTauHypo.TrigTauHypoTool import TrigEFTauMVHypoToolFromDict
    menuCA = MenuSequenceCA(flags, selAcc, HypoToolGen=TrigEFTauMVHypoToolFromDict, isProbe=is_probe_leg)
    return (selAcc , menuCA)


@AccumulatorCache
def tauTrackTwoMVASeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauIso',log)
    name = 'MVA'
    (selAcc , menuCA) = _tauPrecSeq(newflags,name,is_probe_leg)
    return menuCA


@AccumulatorCache
def tauTrackTwoLLPSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauIso',log)
    name = 'LLP'
    (selAcc , menuCA) = _tauPrecSeq(newflags,name,is_probe_leg)
    return menuCA


@AccumulatorCache
def tauTrackLRTSeq(flags, is_probe_leg=False):
    newflags = getFlagsForActiveConfig(flags,'tauLRT',log)
    name = 'LRT'
    (selAcc , menuCA) = _tauPrecSeq(newflags,name,is_probe_leg)
    return menuCA
