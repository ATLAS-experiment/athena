#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory


@AccumulatorCache
def fastTrackingSequenceGenCfg(flags, variant='', is_probe_leg = False):
    """ second step:  tracking....."""
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(variant)
    inViewRoIs = "EMIDRoIs"+variant
    
    # calling the fastTracking Reco algo
    from TriggerMenuMT.HLT.Electron.FastTrackingRecoSequences import fastTracking
    fastTrackingReco, recoFlags = fastTracking(flags, inViewRoIs, variant)
    
    # preparing roiTool
    ViewCreatorCentredOnClusterROITool=CompFactory.ViewCreatorCentredOnClusterROITool
    roiTool = ViewCreatorCentredOnClusterROITool()
    roiTool.AllowMultipleClusters = False # If True: SuperROI mode. If False: highest eT cluster in the L1 ROI
    roiTool.RoisWriteHandleKey = TrigEgammaKeys.fastTrackingRoIContainer
    roiTool.RoIEtaWidth = recoFlags.Tracking.ActiveConfig.etaHalfWidth
    roiTool.RoIPhiWidth = recoFlags.Tracking.ActiveConfig.phiHalfWidth
    if recoFlags.Tracking.ActiveConfig.zedHalfWidth > 0 :
        roiTool.RoIZedWidth = recoFlags.Tracking.ActiveConfig.zedHalfWidth
    viewName="EMFastTracking"+variant    
    fastInDetReco = InViewRecoCA(viewName,
                                 RoITool=roiTool, # view maker args
                                 RequireParentView=True,
                                 InViewRoIs=inViewRoIs,
                                 isProbe=is_probe_leg)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix='IM_'+fastInDetReco.name)

    fastInDetReco.mergeReco(fastTrackingReco)
    selAcc=SelectionCA('ElectronFTF'+variant, isProbe=is_probe_leg)
    selAcc.mergeReco(fastInDetReco, robPrefetchCA=robPrefetchAlg)
    fastElectronHypoAlg = CompFactory.TrigStreamerHypoAlg("ElectronfastTrackingHypo"+variant)
    fastElectronHypoAlg.FeatureIsROI = False
    selAcc.addHypoAlgo(fastElectronHypoAlg)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=acceptAllHypoToolGen,isProbe=is_probe_leg)


def fastTracking_LRTSequenceGenCfg(flags, is_probe_leg=False):
    # This is to call fastElectronMenuSequence for the _LRT variant
    return fastTrackingSequenceGenCfg(flags, variant='_LRT', is_probe_leg=is_probe_leg)
