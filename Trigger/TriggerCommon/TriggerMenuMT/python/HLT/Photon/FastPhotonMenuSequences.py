# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

    
@AccumulatorCache
def fastPhotonSequenceCfg(flags,is_probe_leg=False):
    """Creates secpond step photon sequence"""
    
    TrigEgammaKeys = getTrigEgammaKeys()

    InViewRoIs = "EMIDRoIs"
    # Spawn View on SuperRoI encompassing all clusters found within the L1 RoI
    roiTool = CompFactory.ViewCreatorCentredOnClusterROITool()
    roiTool.AllowMultipleClusters = False # If True: SuperROI mode. If False: highest eT cluster in the L1 ROI
    roiTool.RoisWriteHandleKey = TrigEgammaKeys.fastPhotonRoIContainer
    # not running the tracking here, so do not need to set this size 
    # from the ID Trigger configuration, however, if we want overlap 
    # of the Rois then we would need to use the electron instance size
    # for consistency
    roiTool.RoIEtaWidth = 0.05
    roiTool.RoIPhiWidth = 0.10
    reco = InViewRecoCA("EMPhoton",InViewRoIs=InViewRoIs, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg)


    from TriggerMenuMT.HLT.Photon.FastPhotonRecoSequences import fastPhotonRecoSequence
    reco.mergeReco(fastPhotonRecoSequence(flags, InViewRoIs, "FastPhotonRecoSequence"))
 
    thePhotonHypo = CompFactory.TrigEgammaFastPhotonHypoAlg()    
    thePhotonHypo.Photons = TrigEgammaKeys.fastPhotonContainer
    thePhotonHypo.RunInView=True

    from TrigEgammaHypo.TrigEgammaFastPhotonHypoTool import TrigEgammaFastPhotonHypoToolFromDict

    selAcc = SelectionCA('FastPhotonMenuSequence',isProbe=is_probe_leg)
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(thePhotonHypo)

    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaFastPhotonHypoToolFromDict,isProbe=is_probe_leg)
