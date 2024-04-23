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
    return 'precision' + ('HI' if ion is True else '') + 'PhotonCaloIso'

@AccumulatorCache
def precisionPhotonCaloIsoSequenceGenCfg(flags, name, ion=False, is_probe_leg=False):
    """Creates secpond step photon sequence"""
    
    InViewRoIs = "PrecisionPhotonCaloIsoRoIs"
    hiInfo = 'HI' if ion is True else ''
    TrigEgammaKeys = getTrigEgammaKeys(ion=ion)

    roiTool = CompFactory.ViewCreatorPreviousROITool()
    recoAcc = InViewRecoCA(tag(ion),InViewRoIs=InViewRoIs, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg)

    from TriggerMenuMT.HLT.Photon.PrecisionPhotonCaloIsoRecoSequences import precisionPhotonCaloIsoRecoSequence
    recoAcc.mergeReco(precisionPhotonCaloIsoRecoSequence(flags, InViewRoIs,'PrecisionPhotonCaloIsoRecoSequence'+hiInfo, ion))
   
    selAcc = SelectionCA('PrecisionPhotonCaloIsoMenuSequences'+hiInfo, isProbe=is_probe_leg)
    
    from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigEgammaFSEventDensitySequenceCfg
    selAcc.mergeReco(recoAcc, upSequenceCA= TrigEgammaFSEventDensitySequenceCfg(flags))
    from TrigEgammaHypo.TrigEgammaPrecisionPhotonCaloIsoHypoTool import createTrigEgammaPrecisionPhotonCaloIsoHypoAlg, TrigEgammaPrecisionPhotonCaloIsoHypoToolFromDict
    sequenceOut = TrigEgammaKeys.precisionPhotonIsoContainer
    selAcc.addHypoAlgo(createTrigEgammaPrecisionPhotonCaloIsoHypoAlg(name+tag(ion) +"Hypo", sequenceOut, TrigEgammaKeys.precisionPhotonContainer)) 
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaPrecisionPhotonCaloIsoHypoToolFromDict,isProbe=is_probe_leg)
