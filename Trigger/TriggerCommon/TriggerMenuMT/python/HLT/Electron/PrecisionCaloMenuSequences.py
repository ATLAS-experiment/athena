#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'CaloElectron'

@AccumulatorCache
def precisionCaloSequenceCfg(flags, ion=False, is_probe_leg=False, variant=''):
    """ Creates PrecisionCalo sequence """
    TrigEgammaKeys = getTrigEgammaKeys(variant, ion=ion)
    hiInfo = 'HI' if ion else ''
    # EV creator
    InViewRoIs="PrecisionCaloRoIs"+ variant
    roiTool = CompFactory.ViewCreatorPreviousROITool()
    # Note: This step processes Decision Objects which have followed either Electron reco, Photon reco, or both.
    # For Decision Object which have followed both, there is an ambiguity about which ROI should be used in this
    # merged step. In such cases we break the ambiguity by specifying that the Electron ROI is to be used.
    roiTool.RoISGKey = "HLT_Roi_FastElectron"
    
    recoAcc = InViewRecoCA(tag(ion)+variant,InViewRoIs=InViewRoIs, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg)
    # reco sequence
    from TriggerMenuMT.HLT.Electron.PrecisionCaloRecoSequences import precisionCaloRecoSequence
    recoAcc.mergeReco(precisionCaloRecoSequence(flags, InViewRoIs,'ePrecisionCaloRecoSequence'+hiInfo+variant, ion=ion, variant=variant))
       
    selAcc = SelectionCA('ePrecisionCaloMenuSequence'+hiInfo+variant, isProbe=is_probe_leg)
    pedestalCA = None
    if ion is True:
        # add UE subtraction for heavy ion e/gamma triggers
        # NOTE: UE subtraction requires an average pedestal to be calculated
        # using the full event (FS info), and has to be done outside of the
        # event views in this sequence. the egammaFSHIEventShapeMakerCfg is thus placed
        # in the upSequenceCA before the recoCA.
        from TriggerMenuMT.HLT.HeavyIon.HeavyIonMenuSequences import egammaFSHIEventShapeMakerCfg
        pedestalCA = egammaFSHIEventShapeMakerCfg(flags)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+hiInfo+'_probe'+variant if is_probe_leg else InViewRoIs+hiInfo+variant)

    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg, upSequenceCA=pedestalCA)

    hypoAlg = CompFactory.TrigEgammaPrecisionCaloHypoAlg('Electron' + tag(ion) + 'Hypo' + variant)

    hypoAlg.CaloClusters = TrigEgammaKeys.precisionElectronCaloClusterContainer
    
    selAcc.addHypoAlgo(hypoAlg)
    
    from TrigEgammaHypo.TrigEgammaPrecisionCaloHypoTool import TrigEgammaPrecisionCaloHypoToolFromDict

    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigEgammaPrecisionCaloHypoToolFromDict, isProbe=is_probe_leg)


def precisionCaloSequence_LRTCfg(flags, ion=False, is_probe_leg=False):
    return precisionCaloSequenceCfg(flags, ion=ion, is_probe_leg=is_probe_leg, variant='_LRT')
