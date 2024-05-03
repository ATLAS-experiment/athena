# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

# logger
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Photon'


@AccumulatorCache
def precisionPhotonSequenceGenCfg(flags, ion=False, is_probe_leg=False):
    """ This function creates the PrecisionPhoton sequence"""
    # Prepare first the EventView
    InViewRoIs="PrecisionPhotonRoIs"                                          

    # Configure the reconstruction algorithm sequence
    TrigEgammaKeys = getTrigEgammaKeys(ion = ion)   

    hiInfo = 'HI' if ion is True else ''
    probeInfo = '_probe' if  is_probe_leg is True else ''

    roiTool = CompFactory.ViewCreatorPreviousROITool()
    recoAcc = InViewRecoCA(tag(ion),InViewRoIs=InViewRoIs, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg)

    from TriggerMenuMT.HLT.Photon.PrecisionPhotonRecoSequences import precisionPhotonRecoSequence
    recoAcc.mergeReco(precisionPhotonRecoSequence(flags, InViewRoIs,'PrecisionPhotonRecoSequence'+hiInfo, ion))
    
    selAcc = SelectionCA('PrecisionPhotonMenuSequence'+hiInfo, isProbe=is_probe_leg)

    selAcc.mergeReco(recoAcc)
    sequenceOut = TrigEgammaKeys.precisionPhotonContainer

    from TrigEgammaHypo.TrigEgammaPrecisionPhotonHypoTool import TrigEgammaPrecisionPhotonHypoAlgCfg, TrigEgammaPrecisionPhotonHypoToolFromDict

    selAcc.mergeHypo(TrigEgammaPrecisionPhotonHypoAlgCfg(flags, "Photon"+tag(ion)+"Hypo"+probeInfo, sequenceOut))

    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigEgammaPrecisionPhotonHypoToolFromDict, isProbe=is_probe_leg)
