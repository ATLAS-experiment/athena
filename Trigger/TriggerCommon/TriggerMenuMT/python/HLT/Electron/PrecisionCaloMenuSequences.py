#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys    import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'CaloElectron'

@AccumulatorCache
def precisionCaloMenuSequenceCfg(flags, name=None, ion=False, is_probe_leg=False, variant=''):
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
    if ion is True:
        # add UE subtraction for heavy ion e/gamma triggers
        # NOTE: UE subtraction requires an average pedestal to be calculated
        # using the full event (FS info), and has to be done outside of the
        # event views in this sequence. the egammaFSRecoSequence is thus placed
        # before the precisionCaloInViewSequence.
        from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import egammaFSCaloRecoSequenceCfg
        selAcc.merge(egammaFSCaloRecoSequenceCfg(flags))

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+hiInfo+'_probe'+variant if is_probe_leg else InViewRoIs+hiInfo+variant)

    selAcc.mergeReco(recoAcc, robPrefetchCA=robPrefetchAlg)

    hypoAlg = CompFactory.TrigEgammaPrecisionCaloHypoAlg(name+tag(ion) + 'Hypo' + variant)

    hypoAlg.CaloClusters = TrigEgammaKeys.precisionElectronCaloClusterContainer
    
    selAcc.addHypoAlgo(hypoAlg)
    
    from TrigEgammaHypo.TrigEgammaPrecisionCaloHypoTool import TrigEgammaPrecisionCaloHypoToolFromDict

    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigEgammaPrecisionCaloHypoToolFromDict, isProbe=is_probe_leg)


def precisionCaloMenuSequence(flags, name, is_probe_leg=False, ion=False):
    """ Creates precisionCalo MENU sequence """

    if isComponentAccumulatorCfg():
        return precisionCaloMenuSequenceCfg(flags, name = name, ion = ion, is_probe_leg=is_probe_leg, variant='')
    else:
        return menuSequenceCAToGlobalWrapper(precisionCaloMenuSequenceCfg, flags, name, ion=ion, is_probe_leg=is_probe_leg, variant='')

def precisionCaloMenuSequence_LRT(flags, name, is_probe_leg=False, ion=False):

    if isComponentAccumulatorCfg():
        return precisionCaloMenuSequenceCfg(flags, name = name, ion = ion, is_probe_leg=is_probe_leg, variant='_LRT')
    else:
        return menuSequenceCAToGlobalWrapper(precisionCaloMenuSequenceCfg, flags, name, ion=ion, is_probe_leg=is_probe_leg, variant='_LRT')


