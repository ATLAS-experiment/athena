#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys    import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'CaloPhoton'

@AccumulatorCache
def precisionCaloSequenceGenCfg(flags, ion=False, is_probe_leg=False):
    """ Creates PrecisionCalo sequence """
    TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
    
    hiInfo = 'HI' if ion else ''
    # EV creator
    InViewRoIs="PrecisionCaloRoIs"
    roiTool = CompFactory.ViewCreatorPreviousROITool()
    # Note: This step processes Decision Objects which have followed either Electron reco, Photon reco, or both.
    # For Decision Object which have followed both, there is an ambiguity about which ROI should be used in this
    # merged step. In such cases we break the ambiguity by specifying that the Electron ROI is to be used.
    roiTool.RoISGKey = "HLT_Roi_FastElectron"
    
    recoAcc = InViewRecoCA(tag(ion),InViewRoIs=InViewRoIs, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg)
    # reco sequence
    from TriggerMenuMT.HLT.Photon.PrecisionCaloRecoSequences import precisionCaloRecoSequence
    recoAcc.mergeReco(precisionCaloRecoSequence(flags, InViewRoIs,'gPrecisionCaloRecoSequence'+hiInfo, ion))
       
    selAcc = SelectionCA('gPrecisionCaloMenuSequence'+hiInfo, isProbe=is_probe_leg)
    #TODO mirrors config present is in Electrons - has to be to be unified
    pedestalCA = None
    if ion is True:
        # add UE subtraction for heavy ion e/gamma triggers
        # NOTE: UE subtraction requires an average pedestal to be calculated
        # using the full event (FS info), and has to be done outside of the
        # event views in this sequence. the egammaFSHIEventShapeMaker is thus placed
        # in the upSequenceCA before the recoCA.
        from TriggerMenuMT.HLT.HeavyIon.HeavyIonMenuSequences import egammaFSHIEventShapeMakerCfg
        pedestalCA = egammaFSHIEventShapeMakerCfg(flags)

    selAcc.mergeReco(recoAcc, upSequenceCA=pedestalCA)

    hypoAlg = CompFactory.TrigEgammaPrecisionCaloHypoAlg('Photon' + tag(ion) + 'Hypo')

    hypoAlg.CaloClusters = TrigEgammaKeys.precisionPhotonCaloClusterContainer
    
    selAcc.addHypoAlgo(hypoAlg)
    
    from TrigEgammaHypo.TrigEgammaPrecisionCaloHypoTool import TrigEgammaPrecisionCaloHypoToolFromDict

    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigEgammaPrecisionCaloHypoToolFromDict, isProbe=is_probe_leg)
